#include "script_manager/script_manager.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <variant>

#include "event/timeline.hpp"

std::unique_ptr<v8::Platform> script_manager::platform =
    v8::platform::NewDefaultPlatform();

std::unique_ptr<v8::ArrayBuffer::Allocator> script_manager::allocator =
    std::unique_ptr<v8::ArrayBuffer::Allocator>(
        v8::ArrayBuffer::Allocator::NewDefaultAllocator());

v8::Isolate* script_manager::isolate = nullptr;

event_manager* script_manager::event_manager_ = nullptr;

world* script_manager::world_ = nullptr;

script_manager::script_manager(event_manager& em, world& w) {
  if (isolate == nullptr) {
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    // create and enter an isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = allocator.get();
    isolate = v8::Isolate::New(create_params);
    isolate->Enter();
    // create, persist and enter the global context
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> global_context = v8::Context::New(isolate);
    v8::Eternal<v8::Context>(isolate, global_context);
    global_context->Enter();
    // connect the event handler to the global context
    event_manager_ = &em;
    v8::Local<v8::Object> event_handler = v8::Object::New(isolate);
    event_handler->Set(
        v8::String::NewFromUtf8(isolate, "raise"),
        v8::Function::New(isolate, script_manager::raise_callback));
    global_context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "event_handler"), event_handler);
    // connect the world to the global context
    world_ = &w;
    v8::Local<v8::Object> world = v8::Object::New(isolate);
    if (!world
             ->SetAccessor(isolate->GetCurrentContext(),
                           v8::String::NewFromUtf8(isolate, "time"),
                           script_manager::world_time_getter)
             .FromJust()) {
      std::cerr << "error while setting `world.time` getter" << std::endl;
    }
    if (!world
             ->SetAccessor(isolate->GetCurrentContext(),
                           v8::String::NewFromUtf8(isolate, "finished"),
                           script_manager::world_finished_getter,
                           script_manager::world_finished_setter)
             .FromJust()) {
      std::cerr << "error while setting `world.finished` getter/setter"
                << std::endl;
    }
    v8::Local<v8::Object> moving_platform = v8::Object::New(isolate);
    if (!moving_platform
             ->SetAccessor(isolate->GetCurrentContext(),
                           v8::String::NewFromUtf8(isolate, "x"),
                           script_manager::platform_x_getter,
                           script_manager::platform_x_setter)
             .FromJust()) {
      std::cerr << "error while setting `moving_platform.x` getter/setter"
                << std::endl;
    }
    world->Set(v8::String::NewFromUtf8(isolate, "moving_platform"),
               moving_platform);
    global_context->Global()->Set(v8::String::NewFromUtf8(isolate, "world"),
                                  world);
    // add get_time function to the global context
    global_context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "get_time"),
        v8::Function::New(isolate, script_manager::get_time_callback));
  }
}

script_manager::~script_manager() {
  isolate->Exit();
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
}

void script_manager::run_script(std::filesystem::path path) {
  std::fstream source_file(path);
  if (!source_file.good()) {
    std::cerr << "script file is not working" << std::endl;
    return;
  }
  std::string source_string{
      std::istreambuf_iterator<char>(source_file),
      std::istreambuf_iterator<char>(),
  };
  // create local handles
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::String> source_handle =
      v8::String::NewFromUtf8(isolate, source_string.c_str());
  // compile the script
  v8::TryCatch try_catch(isolate);
  v8::Local<v8::Script> script_handle = v8::Script::Compile(source_handle);
  if (script_handle.IsEmpty()) {
    v8::String::Utf8Value error(isolate, try_catch.Exception());
    std::cerr << *error << std::endl;
    return;
  }
  // run the script
  v8::Local<v8::Value> result_handle = script_handle->Run();
  if (result_handle.IsEmpty()) {
    v8::String::Utf8Value error(isolate, try_catch.Exception());
    std::cerr << *error << std::endl;
    return;
  }
}

void script_manager::raise_callback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> event = info[0]->ToObject();
  // extract event paramater from the V8 object
  std::map<std::string, std::variant<std::string, double, bool>> param_map;
  v8::Local<v8::Object> param_handle =
      event->Get(v8::String::NewFromUtf8(isolate, "param"))->ToObject();
  v8::Local<v8::Array> param_keys = param_handle->GetOwnPropertyNames();
  for (uint32_t i = 0; i < param_keys->Length(); i++) {
    v8::Local<v8::Value> key = param_keys->Get(i);
    v8::Local<v8::Value> value = param_handle->Get(key);
    //
    if (value->IsString()) {
      param_map[*v8::String::Utf8Value(key)] =
          *v8::String::Utf8Value(value->ToString());
    } else if (value->IsNumber()) {
      param_map[*v8::String::Utf8Value(key)] = value->NumberValue();
    } else if (value->IsBoolean()) {
      param_map[*v8::String::Utf8Value(key)] = value->BooleanValue();
    }
  }
  //
  std::string type = *v8::String::Utf8Value(
      event->Get(v8::String::NewFromUtf8(isolate, "type")));
  event_manager_->raise_event({type, param_map});
}

void script_manager::get_time_callback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(
      (double)timeline::get_time().time_since_epoch().count());
}

void script_manager::world_time_getter(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set((double)world_->time.time_since_epoch().count());
}

void script_manager::world_finished_getter(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(world_->finished);
}

void script_manager::world_finished_setter(
    v8::Local<v8::Name> property, v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  world_->finished = value->BooleanValue();
}

void script_manager::platform_x_getter(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(world_->moving_platform.x);
}

void script_manager::platform_x_setter(
    v8::Local<v8::Name> property, v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  world_->moving_platform.x = value->NumberValue();
}
