
#include <memory>
#include <rxcpp/rx.hpp>

// behaviorsubject
void behaviorsubjectobervableDo() {
  rxcpp::subjects::behavior<int> behsubject(0);

  auto observable = behsubject.get_observable();
  observable.subscribe([](int v) { printf("1--------%d\n", v); });

  observable.subscribe([](int v) { printf("2--------%d\n", v); });

  auto subscriber = behsubject.get_subscriber();
  subscriber.on_next(1);
  subscriber.on_next(2);

  int n = behsubject.get_value();
  observable.subscribe([](int v) { printf("3--------%d\n", v); });
  printf("Last value .......%d\n", n);
}

// subject
// 编写一个简单的程序，它将以观察者的身份订阅数据，并作为一对订阅者的可观察对象
//观察者及其变体(订阅者)
void subscriberobervableDo() {
  //----- create a subscription object
  auto subscripiton = rxcpp::composite_subscription();
  //----- create a subscription
  auto subscriber = rxcpp::make_subscriber<int>(
      subscripiton,
      [&](int v) {
        printf("OnNext: -- %d\n", v);
        if (v == 3) subscripiton.unsubscribe();
      },
      []() { printf("OnCompleted\n"); });

  rxcpp::observable<>::create<int>([](rxcpp::subscriber<int> s) {
    for (int i = 0; i < 5; ++i) {
      if (!s.is_subscribed()) break;
      s.on_next(i);
    }
    s.on_completed();
  }).subscribe(subscriber);
}

// replayall
void replayobservableDo() {
  auto values = rxcpp::observable<>::interval(std::chrono::milliseconds(50),
                                              rxcpp::observe_on_new_thread())
                    .take(5)
                    .replay();
  // subscribe from the begining
  values.subscribe([](long v) { printf("[1] OnNext:%ld\n", v); },
                   []() { printf("[1] OnCompleted\n"); });

  // start emiting
  values.connect();

  // Wait before subscribing
  rxcpp::observable<>::timer(std::chrono::microseconds(125))
      .subscribe([&](long) {
        values.as_blocking().subscribe(
            [](long v) { printf("[2] OnNext:%ld\n", v); },
            []() { printf("[2] OnComplete\n"); });
      });

  // --------- Wait for two seconds
  rxcpp::observable<>::timer(std::chrono::milliseconds(2000))
      .subscribe([&](long) {});
}

// 在外部创建生产者可观察对象称为热可观察对象
// 我们可以通过调用可观察对象的发布方法将冷可观察对象转换为热可观察对象。将冷可观察对象转换为热可观察对象的结果是，以后的订阅将丢失数据。不管是否有订阅，热可观察对象都会发出数据。
void hotobservableDo() {
  auto eventloop = rxcpp::observe_on_event_loop();
  // -----create a cold observable
  // -----covert cold observable to hot observable
  // -----using .publish()
  auto values = rxcpp::observable<>::interval(std::chrono::microseconds(2))
                    .take(2)
                    .publish();

  // subscribe twice
  values.subscribe_on(eventloop).subscribe(
      [](int v) { printf("[1] OnNext:%d\n", v); },
      []() { printf("[1] OnCompleted\n"); });

  values.subscribe_on(eventloop).subscribe(
      [](int v) { printf("[2] OnNext:%d\n", v); },
      []() { printf("[2] OnCompleted\n"); });

  // ----start emiting values
  values.connect();
  // ----make a blocking to see result
  values.as_blocking().subscribe();
  // waiting for two seconds
  rxcpp::observable<>::timer(std::chrono::milliseconds(2000))
      .subscribe([&](long) {});
}

//一开始有个订阅者，两秒后又有个订阅者，这两个序列按照自己的节奏走的，不同步。每个流进行都会从interval的0开始。
//在内部创建了生产者的任何可观测对象都称为冷可观察对象。
void coldobservableDo() {
  // -----Get a coordination
  auto eventloop = rxcpp::observe_on_event_loop();
  // -----Create a Cold observable
  auto values = rxcpp::observable<>::interval(std::chrono::seconds(2)).take(2);
  // -----Subscribe Twice
  values.subscribe_on(eventloop).subscribe(
      [](int v) { printf("[1] OnNext:%d\n", v); },
      []() { printf("[1] OnCompleted\n"); });

  values.subscribe_on(eventloop).subscribe(
      [](int v) { printf("[2] OnNext:%d\n", v); },
      []() { printf("[2] OnCompleted\n"); });

  // ---make a blocking subscription to see the result
  values.as_blocking().subscribe();

  // Waiting for two seconds
  rxcpp::observable<>::timer(std::chrono::milliseconds(2000))
      .subscribe([&](long) {});
}

int main(void) {
  // coldobservableDo();

  // hotobservableDo();

  // replayobservableDo();

  // subscriberobervableDo();

  behaviorsubjectobervableDo();
  return 0;
}