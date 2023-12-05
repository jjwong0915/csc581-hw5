#ifndef SCRIPT_MANAGER_HPP
#define SCRIPT_MANAGER_HPP

#include <v8/libplatform/libplatform.h>
#include <v8/v8.h>

#include <filesystem>
#include <memory>
#include <string>

#include "event/event_manager.hpp"
#include "world/world.hpp"

class script_manager {
  static std::unique_ptr<v8::Platform> platform;
  static std::unique_ptr<v8::ArrayBuffer::Allocator> allocator;
  static v8::Isolate* isolate;
  static event_manager* event_manager_;
  static world* world_;
  static void raise_callback(const v8::FunctionCallbackInfo<v8::Value>& info);
  static void get_time_callback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void mainc_vy_getter(v8::Local<v8::Name> property,
                              const v8::PropertyCallbackInfo<v8::Value>& info);
  static void mainc_vy_setter(v8::Local<v8::Name> property,
                              v8::Local<v8::Value> value,
                              const v8::PropertyCallbackInfo<void>& info);

 public:
  script_manager(event_manager& em, world& w);
  ~script_manager();
  void run_script(std::filesystem::path path);
};

#endif
