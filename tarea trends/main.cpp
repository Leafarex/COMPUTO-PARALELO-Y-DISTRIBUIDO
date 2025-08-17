#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>

class RandomSumTask {
public:
    RandomSumTask(int count, int min_v, int max_v)
        : count_(count), min_(min_v), max_(max_v) {
        numbers_.reserve(count_);
    }

    // Genera 'count_' números aleatorios en [min_, max_] y calcula la suma.
    void run() {
        numbers_.clear();
        sum_ = 0;

        std::random_device rd;
        auto seed = static_cast<std::mt19937::result_type>(
            rd() ^
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
            static_cast<uint64_t>(
                std::chrono::high_resolution_clock::now().time_since_epoch().count()
            )
        );
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(min_, max_);

        for (int i = 0; i < count_; ++i) {
            int r = dist(gen);
            numbers_.push_back(r);
            sum_ += r;
        }
    }

    unsigned long long sum() const { return sum_; }
    const std::vector<int>& numbers() const { return numbers_; }

private:
    int count_;
    int min_, max_;
    std::vector<int> numbers_;
    unsigned long long sum_ = 0;
};

int main() {
    constexpr int kNumThreads = 10;
    constexpr int kNumsPerThread = 100;
    constexpr int kMin = 1;
    constexpr int kMax = 1000;

    // Tareas (una por thread)
    std::vector<RandomSumTask> tasks;
    tasks.reserve(kNumThreads);
    for (int i = 0; i < kNumThreads; ++i) {
        tasks.emplace_back(kNumsPerThread, kMin, kMax);
    }

    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    std::vector<std::thread::id> ids(kNumThreads);

    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([i, &tasks, &ids]() {
            tasks[i].run();
            ids[i] = std::this_thread::get_id();
        });
    }
    for (auto& t : threads) t.join();

    // Imprimir resultados
    for (int i = 0; i < kNumThreads; ++i) {
        std::cout << "Thread " << i
                  << " (id " << ids[i] << ") -> suma = "
                  << tasks[i].sum() << ", numeros aleatorios = (";
        const auto& vec = tasks[i].numbers();
        for (size_t j = 0; j < vec.size(); ++j) {
            std::cout << vec[j] << (j + 1 < vec.size() ? ", " : "");
        }
        std::cout << ")\n";
    }

    // Thread con mayor suma
    int idx_max = static_cast<int>(std::distance(
        tasks.begin(),
        std::max_element(tasks.begin(), tasks.end(),
                         [](const RandomSumTask& a, const RandomSumTask& b) {
                             return a.sum() < b.sum();
                         })
    ));

    std::cout << "\nEl thread con puntuacion mas alta es el " << idx_max
              << " (id " << ids[idx_max] << ") con suma = "
              << tasks[idx_max].sum() << ".\n";

    return 0;
}
