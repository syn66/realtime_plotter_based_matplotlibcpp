#include "plotter.h"


int main() {
    std::unique_ptr<RealTimePlot::Plotter> plotter_ptr = std::make_unique<RealTimePlot::Plotter>();

    plotter_ptr->set_subplot_layout({1,2});
    plotter_ptr->set_axis({0,100}, {-10, 10});


    std::vector<double> x, y;
    x.emplace_back(3.0);
    y.emplace_back(2.0);


    while (true) {
        plotter_ptr->get_realtime_plotter<1>(true)->plot<RealTimePlot::TYPE::NAMED_LINE>(x, y, "test_1", "r-.");

        std::map<std::string, std::string> key_words;
        key_words.emplace("color", "red");
        key_words.emplace("marker", "D");
        plotter_ptr->get_realtime_plotter<2>(true)->plot<RealTimePlot::TYPE::SCATTER>(x, y, key_words, 3.0);

        usleep(10 * 1000);
        x.emplace_back(x.back() + 0.2);
        y.emplace_back(y.back() + 0.1);
    }

    return 0;
}