#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
#include <numeric>

class RandomSumTask {
public:
    RandomSumTask(int numbersToGenerateParam, int randomMinInclusiveParam, int randomMaxInclusiveParam)
        : numbersToGenerate(numbersToGenerateParam),
          randomMinInclusive(randomMinInclusiveParam),
          randomMaxInclusive(randomMaxInclusiveParam) {
        generatedNumbers.reserve(numbersToGenerate);
    }

    // Genera 'numbersToGenerate' números aleatorios en [randomMinInclusive, randomMaxInclusive] y calcula la suma.
    void run() {
        generatedNumbers.clear();
        sumOfNumbers = 0;

        std::random_device randomDevice;
        auto rngSeed = static_cast<std::mt19937::result_type>(
            randomDevice() ^
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
            static_cast<uint64_t>(
                std::chrono::high_resolution_clock::now().time_since_epoch().count()
            )
        );
        std::mt19937 rngEngine(rngSeed);
        std::uniform_int_distribution<int> distribution(randomMinInclusive, randomMaxInclusive);

        for (int valueIndex = 0; valueIndex < numbersToGenerate; ++valueIndex) {
            int randomValue = distribution(rngEngine);
            generatedNumbers.push_back(randomValue);
            sumOfNumbers += randomValue;
        }
    }

    unsigned long long getSum() const { return sumOfNumbers; }
    const std::vector<int>& getGeneratedNumbers() const { return generatedNumbers; }

private:
    int numbersToGenerate;
    int randomMinInclusive, randomMaxInclusive;
    std::vector<int> generatedNumbers;
    unsigned long long sumOfNumbers = 0;
};

int main() {
    constexpr int THREAD_COUNT = 10;
    constexpr int NUMBERS_PER_THREAD = 100;
    constexpr int RANDOM_MIN_INCLUSIVE = 1;
    constexpr int RANDOM_MAX_INCLUSIVE = 1000;

    // Tareas (una por thread)
    std::vector<RandomSumTask> workerTasks;
    workerTasks.reserve(THREAD_COUNT);
    for (int threadIndex = 0; threadIndex < THREAD_COUNT; ++threadIndex) {
        workerTasks.emplace_back(NUMBERS_PER_THREAD, RANDOM_MIN_INCLUSIVE, RANDOM_MAX_INCLUSIVE);
    }

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(THREAD_COUNT);
    std::vector<std::thread::id> workerThreadIds(THREAD_COUNT);

    for (int threadIndex = 0; threadIndex < THREAD_COUNT; ++threadIndex) {
        workerThreads.emplace_back([threadIndex, &workerTasks, &workerThreadIds]() {
            workerTasks[threadIndex].run();
            workerThreadIds[threadIndex] = std::this_thread::get_id();
        });
    }
    for (auto& threadObj : workerThreads) threadObj.join();

    // Imprimir resultados
    for (int threadIndex = 0; threadIndex < THREAD_COUNT; ++threadIndex) {
        std::cout << "Thread " << threadIndex
                  << " (id " << workerThreadIds[threadIndex] << ") -> suma = "
                  << workerTasks[threadIndex].getSum() << ", numeros aleatorios = (";
        const auto& numbersInThread = workerTasks[threadIndex].getGeneratedNumbers();
        for (size_t valueIndex = 0; valueIndex < numbersInThread.size(); ++valueIndex) {
            std::cout << numbersInThread[valueIndex]
                      << (valueIndex + 1 < numbersInThread.size() ? ", " : "");
        }
        std::cout << ")\n";
    }

    // Thread con mayor suma usando std::sort y tomando el primero
    std::vector<int> indices(THREAD_COUNT);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&](int a, int b) { return workerTasks[a].getSum() > workerTasks[b].getSum(); });
    int indexOfMaxSum = indices.front();

    std::cout << "\nEl thread con puntuacion mas alta es el " << indexOfMaxSum
              << " (id " << workerThreadIds[indexOfMaxSum] << ") con suma = "
              << workerTasks[indexOfMaxSum].getSum() << ".\n";

    return 0;
}

