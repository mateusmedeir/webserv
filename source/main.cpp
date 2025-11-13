#include "Logger.hpp"
#include "StdLogHandler.hpp"
#include "FileLogHandler.hpp"
#include <iostream>
// #include <ctime>

int main(void) {
    // Logger *log = Logger::getInstance(ERROR, new StdLogHandler());
    
    // log->debug("Esta e uma mensagem de debug...");
    // log->error("Esta e uma mensagem de error...");
    // log->info("Esta e uma mensagem de info...");
    // log->warning("Esta e uma mensagem de warning...");

    // Logger *log2 = Logger::getInstance(INFO, new StdLogHandler());

    // log2->debug("Esta e uma mensagem de debug...");
    // log2->error("Esta e uma mensagem de error...");
    // log2->info("Esta e uma mensagem de info...");
    // log2->warning("Esta e uma mensagem de warning...");

    Logger::initLogger(DEBUG, new StdLogHandler());
    Logger::debug("Esta e uma mensagem de debug...");
    Logger::info("Esta e uma mensagem de info...");
    Logger::warning("Esta e uma mensagem de warning...");
    Logger::error("Esta e uma mensagem de erro...");

    Logger::initLogger(DEBUG, new FileLogHandler("app.log"));
    Logger::debug("Testando app.log...");
    Logger::info("Testando app.log...");
    Logger::warning("Testando app.log...");
    Logger::error("Testando app.log...");

    // std::time_t timestamp = std::time(NULL);
    // std::cout << std::ctime(&timestamp) << std::endl;

    return (0);
}