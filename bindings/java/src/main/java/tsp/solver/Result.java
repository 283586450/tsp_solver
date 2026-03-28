package tsp.solver;

public final class Result extends NativeResource {
  Result(long nativeHandle) {
    this.nativeHandle = nativeHandle;
  }

  public Status getStatus() {
    return Status.fromNative(nativeGetStatus(requireHandle()));
  }

  public long getObjective() {
    return nativeGetObjective(requireHandle());
  }

  public int getTourSize() {
    return nativeGetTourSize(requireHandle());
  }

  public int[] getTour() {
    return nativeGetTour(requireHandle());
  }

  @Override
  protected void release(long handle) {
    nativeDestroy(handle);
  }

  private static native void nativeDestroy(long handle);
  private static native int nativeGetStatus(long handle);
  private static native long nativeGetObjective(long handle);
  private static native int nativeGetTourSize(long handle);
  private static native int[] nativeGetTour(long handle);
}
