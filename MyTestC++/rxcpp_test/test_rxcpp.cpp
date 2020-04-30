
#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <rxcpp/rx.hpp>
#include <string>
#include <thread>

namespace rxu = rxcpp::util;

using namespace std;
using namespace rxcpp;

std::mutex console_mutex;

// timer
void timerobservableDo() {
  auto scheduler = rxcpp::observe_on_new_thread();
  auto period = std::chrono::seconds(3);
  auto values = rxcpp::observable<>::timer(period, scheduler).finally([]() {
    std::cout << "The final action, thread id=>" << std::this_thread::get_id()
              << std::endl;
  });

  values.as_blocking().subscribe(
      [](int v) {
        std::cout << "OnNext:" << v << "thread id" << std::this_thread::get_id()
                  << std::endl;
      },
      []() {
        std::cout << "OnCompleted, thread id=>" << std::this_thread::get_id()
                  << std::endl;
      });

  std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
  // 必须在主线程sleep，否则finally中的内容打印不全，因为程序退出后，线程销毁
  std::this_thread::sleep_for(std::chrono::microseconds(100));
}

// buffer
void bufferobservableDo() {
  auto values = rxcpp::observable<>::range(1, 10).buffer(3);
  values.subscribe(
      [](std::vector<int> v) {
        printf("OnNext:{");
        std::for_each(v.begin(), v.end(), [](int a) { printf("%d", a); });
        printf("}\n");
      },
      []() { printf("OnCompleted\n"); });
}

// defer
// 当有人视图链接到指定的可观察对象时，我们调用observable_factory lambda
void deferobservableDo() {
  auto observable_factory = []() {
    return rxcpp::observable<>::range(1, 3).map([](int n) { return n * n; });
  };

  auto ints = rxcpp::observable<>::defer(observable_factory);
  ints.subscribe([](int v) { printf("OnNext:%d\n", v); },
                 []() { printf("OnCompleted\n"); });

  ints.subscribe([](int v) { printf("OnNext:%d\n", v); },
                 []() { printf("OnCompleted\n"); });
}

// tap
void tapobservableDo() {
  auto ints = rxcpp::observable<>::range(1, 3).map([](int n) { return n * n; });
  //-----Apply the top operator....the operator
  //-----will act a filter/debug operator
  auto values = ints.tap([](int v) { printf("Tap ----   OnNext:%d\n", v); },
                         []() { printf("Tap -       OnCompleted\n"); });

  // ---------- Do some action
  values.subscribe([](int v) { printf("Subscribe - OnNext:%d\n", v); },
                   []() { printf("Subscribe - OnCompleted\n"); });
}

// concat和merge
void concatobservableDo() {
  auto o1 = rxcpp::observable<>::range(1, 3);
  auto o2 = rxcpp::observable<>::range(4, 6);
  auto values = o1.concat(o2);
  values.subscribe([](int v) { printf("OnNext:%d\n", v); },
                   []() { printf("OnCompleted\n"); });
}
void mergeobservableDo() {
  auto o1 = rxcpp::observable<>::range(1, 3);
  auto o2 = rxcpp::observable<>::range(4, 6);
  auto values = o1.merge(o2);
  values.subscribe([](int v) { printf("OnNext:%d\n", v); },
                   []() { printf("OnCompleted\n"); });
}

// flat/concatmap的区别
void flatmapobservableDo() {
  std::array<std::string, 4> a = {{"Prassed", "Perter", "Sanjay", "Raju"}};
  auto values = rxcpp::observable<>::iterate(a).flat_map(
      [](std::string v) {
        std::array<std::string, 3> salution = {{"Mr.", "Monsieur", "Sri"}};
        return rxcpp::observable<>::iterate(salution);
      },
      [](std::string f, std::string s) { return s + " " + f; });

  values.subscribe([](std::string f) { std::cout << f << std::endl; },
                   []() { std::cout << "Hello World.." << std::endl; });
}

// 连接可观察的流
void observableWillDo() {
  auto o1 = rxcpp::observable<>::range(1, 3);
  auto o2 = rxcpp::observable<>::range(4, 6);
  auto values = o1.concat(o2);
  values.subscribe([](int v) { printf("OnNext:%d\n", v); },
                   []() { printf("OnCompleted\n"); });
}

// 创建一个发射指定值的observable
void someValueObservable() {
  auto values = rxcpp::observable<>::just("abc");
  values.subscribe([](std::string str) { printf("OnNext:%s\n", str.c_str()); },
                   []() { printf("OnCompleted\n"); });
}

// Take
void takeObservable() {
  auto values = rxcpp::observable<>::range(1, 3);
  auto s1 = values.take(3).map(
      [](int prime) { return std::make_tuple("1:", prime); });

  auto s2 = values.take(3).map(
      [](int prime) { return std::make_tuple("2:", prime); });

  s1.concat(s2).subscribe(rxcpp::util::apply_to(
      [](const char *s, int p) { printf("%s %d\n", s, p); }));
}

// map
void mapObservable() {
  auto ints =
      rxcpp::observable<>::range(1, 10).map([](int n) { return n * n; });
  ints.subscribe([](int v) { printf("OnNext:%d\n", v); },
                 []() { printf("OnCompleted\n"); });
}

// observeon
void observeonCPP() {
  auto values =
      rxcpp::observable<>::range(1, 4).map([](int v) { return v * v; });

  // --------- Emit the current thread details-----
  std::cout << "Main Thread id =>" << std::this_thread::get_id() << std::endl;

  // --------- observe_on another thread----
  // --------- make it blocking to
  values.observe_on(rxcpp::synchronize_new_thread())
      .as_blocking()
      .subscribe(
          [](int v) {
            std::cout << "Observable Thread id =>" << std::this_thread::get_id()
                      << " " << v << std::endl;
          },
          []() { std::cout << "OnCompleted" << std::endl; });

  // print the main thread details
  std::cout << "Main Thread id =>" << std::this_thread::get_id() << std::endl;
}

// subscribeon
void CTDetails(int val = 0) {
  console_mutex.lock();
  std::cout << "Current Thread id =>" << std::this_thread::get_id() << val
            << std::endl;
  console_mutex.unlock();
}

void Yield(bool y) {
  if (y) {
    std::this_thread::yield();
  }
}

void subscribeonCPP() {
  // -------coordination object
  auto coordination = rxcpp::serialize_new_thread();

  // -------retrieve the worker
  auto worker = coordination.create_coordinator().get_worker();

  // -------Create an Observable
  auto values = rxcpp::observable<>::interval(std::chrono::milliseconds(50))
                    .take(5)
                    .replay(coordination);

  // subscribe from the beginning
  worker.schedule([&](const rxcpp::schedulers::schedulable &) {
    values.subscribe([](long v) { CTDetails(v); }, []() { CTDetails(); });
  });

  // wait before subscribing
  worker.schedule(coordination.now() + std::chrono::milliseconds(125),
                  [&](const rxcpp::schedulers::schedulable &) {
                    values.subscribe([](long v) { CTDetails(v); },
                                     []() { CTDetails(); });
                  });

  // Start emitting
  worker.schedule(
      [&](const rxcpp::schedulers::schedulable &) { values.connect(); });
  // Add blocking subscription to see results
  values.as_blocking().subscribe();
}

int main(void) {
  // // create an observable.. a stream of numbers
  // auto observable = rxcpp::observable<>::range(1, 12);

  // // subscrible (only onnext and oncompleted lambda given)
  // observable.subscribe([](int v) { printf("OnNext:%d\n", v); },
  //                      []() { printf("OnCompleted\n"); });

  // // second
  // auto values = rxcpp::observable<>::range(1, 12)
  //                   .filter([](int v) { return v % 2 == 0; })
  //                   .map([](int x) { return x * x; });

  // values.subscribe([](int v) { printf("OnNext:%d\n", v); },
  //                  []() { printf("OnCompleted\n"); });

  // // 从零开始创建可观察对象
  // auto ints = rxcpp::observable<>::create<int>([](rxcpp::subscriber<int> s) {
  //   s.on_next(1);
  //   s.on_next(4);
  //   s.on_next(9);
  //   s.on_completed();
  // });
  // ints.subscribe([](int v) { printf("OnNext:%d\n", v); },
  //                []() { printf("OnCompleted\n"); });

  // observableWillDo();

  // someValueObservable();

  // takeObservable();

  // mapObservable();

  // observeonCPP();

  // subscribeonCPP();

  // flatmapobservableDo();

  // concatobservableDo();

  // mergeobservableDo();

  // tapobservableDo();

  // deferobservableDo();

  bufferobservableDo();

  timerobservableDo();

  return 0;
}
