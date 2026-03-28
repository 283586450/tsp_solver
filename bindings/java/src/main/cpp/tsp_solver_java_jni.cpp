#include "tsp_solver/c_api.h"

#include <cstdint>
#include <jni.h>
#include <limits>
#include <vector>

namespace {

template <typename HandleType> HandleType* from_handle(jlong handle) {
  return reinterpret_cast<HandleType*>(static_cast<std::intptr_t>(handle));
}

template <typename HandleType>
HandleType* require_handle(JNIEnv* env, jlong handle, const char* type_name) {
  if (handle == 0) {
    jclass exception_class = env->FindClass("java/lang/IllegalStateException");
    if (exception_class != nullptr) {
      env->ThrowNew(exception_class, type_name);
    }
    return nullptr;
  }
  return from_handle<HandleType>(handle);
}

void throw_exception(JNIEnv* env, const char* class_name, const char* message) {
  jclass exception_class = env->FindClass(class_name);
  if (exception_class != nullptr) {
    env->ThrowNew(exception_class, message);
  }
}

bool throw_for_error(JNIEnv* env, tsp_solver_error_code_t code, const char* context) {
  switch (code) {
  case TSP_SOLVER_ERROR_OK:
    return false;
  case TSP_SOLVER_ERROR_INVALID_ARGUMENT:
    throw_exception(env, "java/lang/IllegalArgumentException", context);
    return true;
  case TSP_SOLVER_ERROR_OUT_OF_RANGE:
    throw_exception(env, "java/lang/IndexOutOfBoundsException", context);
    return true;
  case TSP_SOLVER_ERROR_INVALID_MODEL:
    throw_exception(env, "java/lang/IllegalStateException", context);
    return true;
  case TSP_SOLVER_ERROR_ALLOCATION_FAILED:
  case TSP_SOLVER_ERROR_INTERNAL_ERROR:
    throw_exception(env, "tsp/solver/SolverNativeException", context);
    return true;
  }

  throw_exception(env, "tsp/solver/SolverNativeException", "unknown native error");
  return true;
}

jlong to_java_handle(const void* handle) {
  return static_cast<jlong>(reinterpret_cast<std::intptr_t>(handle));
}

} // namespace

extern "C" {

JNIEXPORT jstring JNICALL Java_tsp_solver_NativeLibrary_nativeVersionString(JNIEnv* env,
                                                                            jclass) {
  const char* version = tsp_solver_version_string();
  return env->NewStringUTF(version == nullptr ? "" : version);
}

JNIEXPORT jlong JNICALL Java_tsp_solver_Model_nativeCreate(JNIEnv* env, jclass) {
  tsp_solver_model_t* model = nullptr;
  if (throw_for_error(env, tsp_solver_model_create(&model), "failed to create model")) {
    return 0;
  }
  return to_java_handle(model);
}

JNIEXPORT void JNICALL Java_tsp_solver_Model_nativeDestroy(JNIEnv*, jclass,
                                                           jlong handle) {
  tsp_solver_model_destroy(from_handle<tsp_solver_model_t>(handle));
}

JNIEXPORT jint JNICALL Java_tsp_solver_Model_nativeAddNode(JNIEnv* env, jclass,
                                                           jlong handle) {
  tsp_solver_model_t* model =
      require_handle<tsp_solver_model_t>(env, handle, "Model is closed");
  if (model == nullptr) {
    return 0;
  }

  tsp_solver_node_id_t node_id = 0;
  if (throw_for_error(env, tsp_solver_model_add_node(model, &node_id),
                      "failed to add node")) {
    return 0;
  }
  return static_cast<jint>(node_id);
}

JNIEXPORT void JNICALL Java_tsp_solver_Model_nativeSetDistance(JNIEnv* env, jclass,
                                                               jlong handle, jint from,
                                                               jint to,
                                                               jlong distance) {
  tsp_solver_model_t* model =
      require_handle<tsp_solver_model_t>(env, handle, "Model is closed");
  if (model == nullptr) {
    return;
  }

  if (throw_for_error(
          env,
          tsp_solver_model_set_distance(model, static_cast<tsp_solver_node_id_t>(from),
                                        static_cast<tsp_solver_node_id_t>(to),
                                        static_cast<tsp_solver_cost_t>(distance)),
          "failed to set distance")) {
    return;
  }
}

JNIEXPORT void JNICALL Java_tsp_solver_Model_nativeValidate(JNIEnv* env, jclass,
                                                            jlong handle) {
  tsp_solver_model_t* model =
      require_handle<tsp_solver_model_t>(env, handle, "Model is closed");
  if (model == nullptr) {
    return;
  }

  throw_for_error(env, tsp_solver_model_validate(model), "model is incomplete");
}

JNIEXPORT jlong JNICALL Java_tsp_solver_Options_nativeCreate(JNIEnv* env, jclass) {
  tsp_solver_options_t* options = nullptr;
  if (throw_for_error(env, tsp_solver_options_create(&options),
                      "failed to create options")) {
    return 0;
  }
  return to_java_handle(options);
}

JNIEXPORT void JNICALL Java_tsp_solver_Options_nativeDestroy(JNIEnv*, jclass,
                                                             jlong handle) {
  tsp_solver_options_destroy(from_handle<tsp_solver_options_t>(handle));
}

JNIEXPORT void JNICALL Java_tsp_solver_Options_nativeSetTimeLimitMs(JNIEnv* env, jclass,
                                                                    jlong handle,
                                                                    jlong value) {
  tsp_solver_options_t* options =
      require_handle<tsp_solver_options_t>(env, handle, "Options is closed");
  if (options == nullptr) {
    return;
  }

  if (value < 0) {
    throw_exception(env, "java/lang/IllegalArgumentException",
                    "time limit must be >= 0");
    return;
  }

  throw_for_error(
      env,
      tsp_solver_options_set_time_limit_ms(options, static_cast<std::uint64_t>(value)),
      "failed to set time limit");
}

JNIEXPORT void JNICALL Java_tsp_solver_Options_nativeSetRandomSeed(JNIEnv* env, jclass,
                                                                   jlong handle,
                                                                   jlong value) {
  tsp_solver_options_t* options =
      require_handle<tsp_solver_options_t>(env, handle, "Options is closed");
  if (options == nullptr) {
    return;
  }

  if (value < 0) {
    throw_exception(env, "java/lang/IllegalArgumentException",
                    "random seed must be >= 0");
    return;
  }

  throw_for_error(
      env,
      tsp_solver_options_set_random_seed(options, static_cast<std::uint64_t>(value)),
      "failed to set random seed");
}

JNIEXPORT void JNICALL Java_tsp_solver_Options_nativeSetAlgorithm(
    JNIEnv* env, jclass, jlong handle, jint algorithm_value) {
  tsp_solver_options_t* options =
      require_handle<tsp_solver_options_t>(env, handle, "Options is closed");
  if (options == nullptr) {
    return;
  }

  throw_for_error(env,
                  tsp_solver_options_set_algorithm(
                      options, static_cast<tsp_solver_algorithm_t>(algorithm_value)),
                  "failed to set algorithm");
}

JNIEXPORT void JNICALL Java_tsp_solver_Result_nativeDestroy(JNIEnv*, jclass,
                                                            jlong handle) {
  tsp_solver_result_destroy(from_handle<tsp_solver_result_t>(handle));
}

JNIEXPORT jint JNICALL Java_tsp_solver_Result_nativeGetStatus(JNIEnv* env, jclass,
                                                              jlong handle) {
  tsp_solver_result_t* result =
      require_handle<tsp_solver_result_t>(env, handle, "Result is closed");
  if (result == nullptr) {
    return 0;
  }

  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  if (throw_for_error(env, tsp_solver_result_get_status(result, &status),
                      "failed to get result status")) {
    return 0;
  }
  return static_cast<jint>(status);
}

JNIEXPORT jlong JNICALL Java_tsp_solver_Result_nativeGetObjective(JNIEnv* env, jclass,
                                                                  jlong handle) {
  tsp_solver_result_t* result =
      require_handle<tsp_solver_result_t>(env, handle, "Result is closed");
  if (result == nullptr) {
    return 0;
  }

  tsp_solver_cost_t objective = 0;
  if (throw_for_error(env, tsp_solver_result_get_objective(result, &objective),
                      "failed to get result objective")) {
    return 0;
  }
  return static_cast<jlong>(objective);
}

JNIEXPORT jint JNICALL Java_tsp_solver_Result_nativeGetTourSize(JNIEnv* env, jclass,
                                                                jlong handle) {
  tsp_solver_result_t* result =
      require_handle<tsp_solver_result_t>(env, handle, "Result is closed");
  if (result == nullptr) {
    return 0;
  }

  std::size_t size = 0;
  if (throw_for_error(env, tsp_solver_result_get_tour_size(result, &size),
                      "failed to get result tour size")) {
    return 0;
  }

  if (size > static_cast<std::size_t>(std::numeric_limits<jint>::max())) {
    throw_exception(env, "tsp/solver/SolverNativeException",
                    "tour size does not fit in jint");
    return 0;
  }

  return static_cast<jint>(size);
}

JNIEXPORT jintArray JNICALL Java_tsp_solver_Result_nativeGetTour(JNIEnv* env, jclass,
                                                                 jlong handle) {
  tsp_solver_result_t* result =
      require_handle<tsp_solver_result_t>(env, handle, "Result is closed");
  if (result == nullptr) {
    return nullptr;
  }

  std::size_t size = 0;
  if (throw_for_error(env, tsp_solver_result_get_tour_size(result, &size),
                      "failed to get result tour size")) {
    return nullptr;
  }

  if (size > static_cast<std::size_t>(std::numeric_limits<jint>::max())) {
    throw_exception(env, "tsp/solver/SolverNativeException",
                    "tour size does not fit in jint");
    return nullptr;
  }

  std::vector<tsp_solver_node_id_t> tour(size);
  std::size_t written = 0;
  if (size > 0 &&
      throw_for_error(env,
                      tsp_solver_result_get_tour(result, tour.data(), size, &written),
                      "failed to get result tour")) {
    return nullptr;
  }

  jintArray array = env->NewIntArray(static_cast<jsize>(written));
  if (array == nullptr) {
    return nullptr;
  }

  std::vector<jint> values(written);
  for (std::size_t index = 0; index < written; ++index) {
    values[index] = static_cast<jint>(tour[index]);
  }
  env->SetIntArrayRegion(array, 0, static_cast<jsize>(written), values.data());
  return array;
}

JNIEXPORT jlong JNICALL Java_tsp_solver_Solver_nativeSolve(JNIEnv* env, jclass,
                                                           jlong model_handle,
                                                           jlong options_handle) {
  tsp_solver_model_t* model =
      require_handle<tsp_solver_model_t>(env, model_handle, "Model is closed");
  tsp_solver_options_t* options =
      require_handle<tsp_solver_options_t>(env, options_handle, "Options is closed");
  if (model == nullptr || options == nullptr) {
    return 0;
  }

  tsp_solver_result_t* result = nullptr;
  if (throw_for_error(env, tsp_solver_solve(model, options, &result),
                      "failed to solve model")) {
    return 0;
  }
  return to_java_handle(result);
}

} // extern "C"
