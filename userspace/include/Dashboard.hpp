/**
 * @file Dashboard.hpp
 * @brief Real-time ncurses terminal dashboard for VSensor.
 */

#pragma once

#include "CircularBuffer.hpp"
#include "SensorData.hpp"
#include "StatsEngine.hpp"
#include "AlertManager.hpp"
#include "AppConfig.hpp"

#include <ncurses.h>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * @brief ncurses-based real-time monitoring dashboard.
 *
 * Layout:
 * ┌──────────────────────────────────────────┐
 * │  VSensor Monitor  │  timestamp           │
 * ├──────────────────────────────────────────┤
 * │  Temperature: 37.4 °C  [ALERT if > 75]  │
 * │  CPU Load:    62.1 %                     │
 * ├──────────────────────────────────────────┤
 * │  Statistics: Min / Max / Avg             │
 * ├──────────────────────────────────────────┤
 * │  Temperature History  (ASCII graph)      │
 * ├──────────────────────────────────────────┤
 * │  [q] Quit                                │
 * └──────────────────────────────────────────┘
 */
class Dashboard {
public:
    Dashboard(CircularBuffer<SensorData>& buffer,
              const StatsEngine&          stats,
              const AlertManager&         alerts,
              const AppConfig&            config)
        : buffer_(buffer), stats_(stats), alerts_(alerts), config_(config) {}

    /** @brief Initialize ncurses. Call before render(). */
    void init() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE); // non-blocking getch()
        curs_set(0);
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_GREEN,  COLOR_BLACK); // normal values
            init_pair(2, COLOR_RED,    COLOR_BLACK); // alert values
            init_pair(3, COLOR_CYAN,   COLOR_BLACK); // headers
            init_pair(4, COLOR_YELLOW, COLOR_BLACK); // labels
        }
    }

    /**
     * @brief Render one frame of the dashboard.
     * @return false if the user pressed 'q' to quit.
     */
    bool render() {
        int ch = getch();
        if (ch == 'q' || ch == 'Q')
            return false;

        clear();
        int row = 0;
        drawHeader(row);
        row += 2;
        drawCurrentValues(row);
        row += 3;
        drawStats(row);
        row += 4;
        drawGraph(row);
        row += 12;
        drawFooter(row);
        refresh();
        return true;
    }

    /** @brief Restore terminal state. Call on shutdown. */
    void cleanup() {
        endwin();
    }

private:
    CircularBuffer<SensorData>& buffer_;
    const StatsEngine&          stats_;
    const AlertManager&         alerts_;
    const AppConfig&            config_;

    void drawHeader(int row) {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(row, 2, "=== VSensor Real-Time Monitor ===");
        attroff(COLOR_PAIR(3) | A_BOLD);

        // Current time
        auto now  = std::chrono::system_clock::now();
        auto tt   = std::chrono::system_clock::to_time_t(now);
        std::ostringstream ts;
        ts << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
        mvprintw(row, 40, "%s", ts.str().c_str());

        mvprintw(row + 1, 2,
                 "Device: %-20s  Interval: %d ms",
                 config_.device_path.c_str(),
                 config_.interval_ms);
    }

    void drawCurrentValues(int row) {
        if (buffer_.empty()) {
            mvprintw(row, 2, "Waiting for data...");
            return;
        }

        SensorData latest = buffer_.latest();
        bool tempAlert = alerts_.checkTemp(latest.temperature);
        bool loadAlert = alerts_.checkLoad(latest.cpu_load);

        attron(COLOR_PAIR(4));
        mvprintw(row, 2, "Temperature : ");
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(tempAlert ? 2 : 1) | A_BOLD);
        printw("%.1f C", latest.temperature);
        if (tempAlert) printw("  *** HIGH TEMP ALERT ***");
        attroff(COLOR_PAIR(tempAlert ? 2 : 1) | A_BOLD);

        attron(COLOR_PAIR(4));
        mvprintw(row + 1, 2, "CPU Load    : ");
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(loadAlert ? 2 : 1) | A_BOLD);
        printw("%.1f %%", latest.cpu_load);
        if (loadAlert) printw("  *** HIGH LOAD ALERT ***");
        attroff(COLOR_PAIR(loadAlert ? 2 : 1) | A_BOLD);

        // Inline bar chart
        attron(COLOR_PAIR(3));
        mvprintw(row + 2, 2, "  Temp [");
        attroff(COLOR_PAIR(3));
        drawBar(row + 2, 10, latest.temperature, 85.0f, 40, tempAlert);
        attron(COLOR_PAIR(3));
        printw("] %.0f%%", (latest.temperature / 85.0f) * 100.0f);
        attroff(COLOR_PAIR(3));

        attron(COLOR_PAIR(3));
        mvprintw(row + 3, 2, "  Load [");
        attroff(COLOR_PAIR(3));
        drawBar(row + 3, 10, latest.cpu_load, 100.0f, 40, loadAlert);
        attron(COLOR_PAIR(3));
        printw("] %.0f%%", latest.cpu_load);
        attroff(COLOR_PAIR(3));
    }

    void drawBar(int row, int col, float value, float max,
                 int width, bool alert) {
        int filled = static_cast<int>((value / max) * width);
        filled = std::min(filled, width);
        attron(COLOR_PAIR(alert ? 2 : 1));
        move(row, col);
        for (int i = 0; i < width; ++i)
            addch(i < filled ? '#' : '-');
        attroff(COLOR_PAIR(alert ? 2 : 1));
    }

    void drawStats(int row) {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(row, 2, "-- Statistics (last %zu samples) --",
                 buffer_.size());
        attroff(COLOR_PAIR(3) | A_BOLD);

        mvprintw(row + 1, 2,
                 "Temperature: Min=%.1f C  Max=%.1f C  Avg=%.1f C",
                 stats_.minTemp(), stats_.maxTemp(), stats_.avgTemp());
        mvprintw(row + 2, 2,
                 "CPU Load  : Min=%.1f%%  Max=%.1f%%  Avg=%.1f%%",
                 stats_.minLoad(), stats_.maxLoad(), stats_.avgLoad());
        mvprintw(row + 3, 2,
                 "Thresholds : Temp=%.1f C  Load=%.1f%%",
                 alerts_.getTempThreshold(),
                 alerts_.getLoadThreshold());
    }

    void drawGraph(int row) {
        static constexpr int GRAPH_HEIGHT = 8;
        static constexpr int GRAPH_WIDTH  = 60;

        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(row, 2, "-- Temperature History (last 60 readings) --");
        attroff(COLOR_PAIR(3) | A_BOLD);

        auto readings = buffer_.range(GRAPH_WIDTH);
        if (readings.empty()) return;

        float lo = 20.0f, hi = 85.0f;
        auto  rng = hi - lo;

        for (int h = GRAPH_HEIGHT - 1; h >= 0; --h) {
            move(row + 1 + (GRAPH_HEIGHT - 1 - h), 2);
            float threshold = lo + (rng * (h + 1)) / GRAPH_HEIGHT;
            attron(COLOR_PAIR(4));
            printw("%4.0f|", threshold);
            attroff(COLOR_PAIR(4));

            for (std::size_t i = 0; i < readings.size(); ++i) {
                float level = lo + (rng * h) / GRAPH_HEIGHT;
                float above = lo + (rng * (h + 1)) / GRAPH_HEIGHT;
                float val   = readings[i].temperature;
                bool  alert = alerts_.checkTemp(val);

                attron(COLOR_PAIR(alert ? 2 : 1));
                if (val >= level && val < above)
                    addch('*');
                else if (val >= above)
                    addch('|');
                else
                    addch(' ');
                attroff(COLOR_PAIR(alert ? 2 : 1));
            }
        }
        // X-axis
        attron(COLOR_PAIR(4));
        mvprintw(row + 1 + GRAPH_HEIGHT, 2, "  °C +");
        for (int i = 0; i < GRAPH_WIDTH; ++i) addch('-');
        attroff(COLOR_PAIR(4));
    }

    void drawFooter(int row) {
        attron(A_DIM);
        mvprintw(row, 2, "[q] Quit  |  Log: %s", config_.logfile.c_str());
        attroff(A_DIM);
    }
};
