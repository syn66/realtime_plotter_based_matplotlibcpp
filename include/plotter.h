#pragma once

#include <memory>
#include "matplotlibcpp.h"

namespace RealTimePlot {

enum class TYPE {SCATTER, LINE, NAMED_LINE, UPDATE};

template<class Numeric>
struct Param
{
    std::vector<Numeric> x_;
    std::vector<Numeric> y_;
    double size_ = 1.0;
    std::string name_ = "";
    std::string format_ = "";
    std::map<std::string, std::string> keyword_ = {};

    //scatter
    Param(const std::vector<Numeric>& x,
          const std::vector<Numeric>& y,
          const std::map<std::string, std::string>& keyword,
          const double& size)
        : x_(x),
        y_(y),
        keyword_(keyword),
        size_(size) {};

    // line with keyword format
    Param(const std::vector<Numeric>& x,
          const std::vector<Numeric>& y,
          const std::map<std::string, std::string>& keyword)
        : x_(x),
        y_(y),
        keyword_(keyword){};

    //line with string format
    Param(const std::vector<Numeric>& x,
          const std::vector<Numeric>& y,
          const std::string& format)
        : x_(x),
        y_(y),
        format_(format){};

    //named line with format
    Param(const std::vector<Numeric>& x,
          const std::vector<Numeric>& y,
          const std::string& legend_name,
          const std::string& format)
        : x_(x),
        y_(y),
        format_(format),
        name_(legend_name){};

    //line with out format
    Param(const std::vector<Numeric>& x,
          const std::vector<Numeric>& y)
        : x_(x),
        y_(y) {};
};


class Plotter
{
private:

    template<class Numeric>
    class base_plot
    {
    public:
        base_plot() = default;
        virtual ~base_plot() = default;

        void set_conf(bool subplot,
                      double fre) {
            subplot_ = subplot;
            fre_ = fre;
        }

        virtual bool plot(const Param<Numeric>&& param) = 0;

        bool subplot_;
        double fre_;
    };

    template<class Numeric>
    class realtime_plot_scatter : public base_plot<Numeric>
    {
    public:
        realtime_plot_scatter() = default;
        ~realtime_plot_scatter() = default;

        bool plot(const Param<Numeric>&& param) override {
            matplotlibcpp::cla();
            matplotlibcpp::scatter(param.x_, param.y_, param.size_, param.keyword_);
            matplotlibcpp::pause(this->fre_);
            return true;
        }
    };

    template<class Numeric>
    class realtime_plot_line : public base_plot<Numeric>
    {
    public:
        realtime_plot_line() = default;
        ~realtime_plot_line() = default;

        bool plot(const Param<Numeric>&& param) override {
            matplotlibcpp::cla();
            matplotlibcpp::plot(param.x_, param.y_, param.format_);
            matplotlibcpp::pause(this->fre_);
            return true;
        }
    };

    template<class Numeric>
    class realtime_namedplot_line : public base_plot<Numeric>
    {
    public:
        realtime_namedplot_line() = default;
        ~realtime_namedplot_line() = default;

        bool plot(const Param<Numeric>&& param) override {
            matplotlibcpp::cla();
            matplotlibcpp::named_plot(param.name_, param.x_, param.y_, param.format_);
            matplotlibcpp::legend();
            matplotlibcpp::pause(this->fre_);
            return true;
        }
    };

    template<class Numeric>
    class realtime_update_plot : public base_plot<Numeric>
    {
    public:
        realtime_update_plot() = default;
        ~realtime_update_plot() = default;

        bool plot(const Param<Numeric>&& param) override {
            if (!update_plot_ptr_.get()) 
                update_plot_ptr_.reset(new matplotlibcpp::Plot(param.name_, param.x_, param.y_, param.format_));
            matplotlibcpp::legend();
            update_plot_ptr_->update(param.x_, param.y_);
            matplotlibcpp::pause(this->fre_);
            return true;
        }

    private:
        std::unique_ptr<matplotlibcpp::Plot> update_plot_ptr_;
    };


    class Function
    {
    public:
        Function(const std::size_t id, bool subplot, double fre)
                : id_(id), subplot_(subplot), fre_(fre) {
            std::cout << "构造 Function"<<'\n';
        };
        ~Function() {
            for (const auto& p : plot_ptr_pool_) {
                delete p.second;
            }
            plot_ptr_pool_.clear();
            std::cout << "析构 Function"<<'\n';
        };

        template<TYPE T, std::size_t idx = 1, typename... Args>
        bool plot(Args&&... args) {
            if (!subplot_) matplotlibcpp::figure(id_);
            else matplotlibcpp::subplot(static_cast<long>(layout_.first), 
                                        static_cast<long>(layout_.second), 
                                        static_cast<long>(id_));
            return set_plot(T, idx)->plot({args...});
        }

    private:

        base_plot<double>* set_plot(TYPE type, std::size_t idx) {
            auto && ptr = plot_ptr_pool_[idx];
            if (!ptr) {
                switch (type)
                {
                case TYPE::SCATTER:
                    ptr = new realtime_plot_scatter<double>();
                    break;
                case TYPE::LINE:
                    ptr = new realtime_plot_line<double>();
                    break;
                case TYPE::NAMED_LINE:
                    ptr = new realtime_namedplot_line<double>();
                    break;
                case TYPE::UPDATE:
                    matplotlibcpp::xlim(Plotter::x_lim_.first, Plotter::x_lim_.second);
                    matplotlibcpp::ylim(Plotter::y_lim_.first, Plotter::y_lim_.second);
                    ptr = new realtime_update_plot<double>();
                    break;
                default:
                    break;
                }
                ptr->set_conf(subplot_, fre_);
            }
            return ptr;
        }

    private:
        std::unordered_map<std::size_t,base_plot<double>*> plot_ptr_pool_;
        bool subplot_;
        std::size_t id_;
        double fre_;
    };

public:
    Plotter() = default;
    ~Plotter() {
        func_pool_.clear();
        matplotlibcpp::close();
    }

    template<std::size_t id>
    std::shared_ptr<Function> get_realtime_plotter(bool subplot = false, double fre = 0.1) {
        return get_plot_impl<id>(subplot, fre);
    }

    void set_subplot_layout(const std::pair<long, long>&& layout) {
        layout_ = std::move(layout);
        std::cout << layout_.first << " " << layout_.second << '\n';
    }

    //针对 update方法axis无法auto缩放
    void set_axis(const std::pair<double, double>&& x_lim,
                  const std::pair<double, double>&& y_lim) {
        x_lim_ = std::move(x_lim);
        y_lim_ = std::move(y_lim);
    }

    static std::pair<long, long> layout_;
    static std::pair<double, double> x_lim_;
    static std::pair<double, double> y_lim_;

private:

    template<std::size_t id> //todo weak_ptr with shared_ptr
    std::shared_ptr<Function> get_plot_impl(bool subplot, double fre) {
        auto&& pfunc = func_pool_[id];
        if (!pfunc) {
            if (subplot) {
                if (id < 0 && id > layout_.first * layout_.second) 
                    throw std::runtime_error("subplot num is wrong");
            }
            pfunc.reset(new Function(id, subplot, fre));
        }
        return pfunc;
    }

private:
    std::unordered_map<std::size_t, std::shared_ptr<Function>> func_pool_;
};

std::pair<long, long>  Plotter::layout_;
std::pair<double, double> Plotter::x_lim_;
std::pair<double, double> Plotter::y_lim_;

}