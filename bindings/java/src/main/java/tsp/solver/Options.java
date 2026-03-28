package tsp.solver;

public final class Options extends NativeResource {
  public Options() {
    NativeLibrary.load();
    nativeHandle = nativeCreate();
  }

  public void setTimeLimitMs(long value) {
    nativeSetTimeLimitMs(requireHandle(), value);
  }

  public void setRandomSeed(long value) {
    nativeSetRandomSeed(requireHandle(), value);
  }

  public void setAlgorithm(Algorithm algorithm) {
    if (algorithm == null) {
      throw new IllegalArgumentException("algorithm must not be null");
    }
    nativeSetAlgorithm(requireHandle(), algorithm.nativeValue());
  }

  @Override
  protected void release(long handle) {
    nativeDestroy(handle);
  }

  private static native long nativeCreate();
  private static native void nativeDestroy(long handle);
  private static native void nativeSetTimeLimitMs(long handle, long value);
  private static native void nativeSetRandomSeed(long handle, long value);
  private static native void nativeSetAlgorithm(long handle, int algorithmValue);
}
