#include <tm1637.h>
#include <tm1637_sayer.h>
#include <tm1637_digits.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

// If no GPIO lib is declared on command line, use GPIOD by default
#if !defined(GPIO_LIB)
#define GPIO_LIB GpioGPIOD
#endif

int main(int argc, char**argv) {
    po::options_description user_options(
        "tm1637_say utility - \u00A9 2023 Neil Davis\n"\
        "See LICENSE at https://github.com/neildavis/lib_tm1637_rpi/blob/main/LICENSE\n\n"\
        "Usage: tm1637_say [OPTIONS] <message>\n\n"
        "Available OPTIONS"
        );
    user_options.add_options()
        ("help,h", "Show usage information")
        ("scl,c", po::value<int>()->default_value(3), "GPIO pin to use for clock")
        ("sda,d", po::value<int>()->default_value(2), "GPIO pin to use for data")
        ("delay-time,t", po::value<int>()->default_value(250), "Delay time between characters (ms)")
        ("count,n", po::value<int>()->default_value(1), "Repeat the message <arg> number of times")
    ;
    po::options_description hidden_options("Hidden options");
    hidden_options.add_options()
        ("message,m", po::value<std::string>(), "Message to display on the TM1637 device")
    ;

    po::options_description all_options("All options");
    all_options.add(user_options).add(hidden_options);
 
    po::positional_options_description pod;
    pod.add("message", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
        .options(all_options)
        .positional(pod).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0 || vm.count("message") == 0) {
        std::cout << user_options << "\n";
        return 1;
    }

    int pinSDA = vm["sda"].as<int>();
    int pinSCL = vm["scl"].as<int>();
    int delay_ms = vm["delay-time"].as<int>();
    int count = vm["count"].as<int>();
    
    auto tm1637 = std::unique_ptr<tm1637::Device>(new tm1637::Device(pinSCL, pinSDA, tm1637::GPIO_LIB));
    tm1637::Sayer sayer(tm1637);
    sayer.begin(vm["message"].as<std::string>());
    for (int i = 0; i < count; i++) {
        while (sayer.next()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        sayer.reset();
    }
    sayer.clear();

    return 0;
}