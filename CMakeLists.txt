cmake_minimum_required(VERSION 3.8)
project(test_cpp_and_threads CXX C)

set(test_files #single source per test
atomic.cpp
callable_object.cpp
call_once.cpp
condition_variable.cpp
condition_variable_and_threads.cpp
data_race_and_mutex.cpp
deadlock.cpp
future_promise_async.cpp
hello_threads.cpp
locks_and_mutex.cpp
packaged_task.cpp
recursive_mutex.cpp
threads_exception_management.cpp
thread_local.cpp
thread_management.cpp
time_constrain.cpp
unique_lock_lazy_initialization.cpp
operator_override.cpp
pin_thread_to_core.cpp
task_thread.cpp
produce_consume.cpp
produce_consume_fast.cpp
)

foreach(file_in_test_files ${test_files})
    get_filename_component(name ${file_in_test_files} NAME_WE)
    add_executable(${name} ${file_in_test_files})
endforeach(file_in_test_files)

add_executable(test_threadpool threadpool_example.cpp ThreadPool.h)
