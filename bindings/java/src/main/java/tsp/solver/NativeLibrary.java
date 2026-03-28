package tsp.solver;

public final class NativeLibrary {
  private static volatile boolean loaded;

  private NativeLibrary() {}

  private static native String nativeVersionString();

  static void ensureVersionMatch(String javaVersion, String nativeVersion) {
    if (javaVersion.equals(nativeVersion)) {
      return;
    }
    throw new IllegalStateException(
        "Java binding version " + javaVersion
            + " does not match native library version " + nativeVersion
            + ". Use the jar and native bundle from the same tsp_solver release.");
  }

  public static void load() {
    load(Version.STRING);
  }

  static void load(String expectedVersion) {
    if (loaded) {
      ensureVersionMatch(expectedVersion, nativeVersionString());
      return;
    }

    synchronized (NativeLibrary.class) {
      if (loaded) {
        ensureVersionMatch(expectedVersion, nativeVersionString());
        return;
      }

      String propertyPath = System.getProperty("tsp.solver.library.path");
      String envPath = System.getenv("TSP_SOLVER_JAVA_LIBRARY_PATH");
      UnsatisfiedLinkError failure = null;

      try {
        if (propertyPath != null && !propertyPath.isEmpty()) {
          System.load(propertyPath);
        } else if (envPath != null && !envPath.isEmpty()) {
          System.load(envPath);
        } else {
          System.loadLibrary("tsp_solver_java_jni");
        }
        ensureVersionMatch(expectedVersion, nativeVersionString());
        loaded = true;
        return;
      } catch (UnsatisfiedLinkError error) {
        failure = error;
      }

      StringBuilder message = new StringBuilder("Unable to load Java JNI bridge");
      if (propertyPath != null && !propertyPath.isEmpty()) {
        message.append(" from system property tsp.solver.library.path=").append(propertyPath);
      } else if (envPath != null && !envPath.isEmpty()) {
        message.append(" from TSP_SOLVER_JAVA_LIBRARY_PATH=").append(envPath);
      } else {
        message.append(" via System.loadLibrary(tsp_solver_java_jni)");
      }

      UnsatisfiedLinkError wrapped = new UnsatisfiedLinkError(message.toString());
      wrapped.initCause(failure);
      throw wrapped;
    }
  }
}
