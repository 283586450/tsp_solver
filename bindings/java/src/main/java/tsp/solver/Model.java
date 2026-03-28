package tsp.solver;

public final class Model extends NativeResource {
  public Model() {
    NativeLibrary.load();
    nativeHandle = nativeCreate();
  }

  public int addNode() {
    return nativeAddNode(requireHandle());
  }

  public void setDistance(int from, int to, long distance) {
    nativeSetDistance(requireHandle(), from, to, distance);
  }

  public void validate() {
    nativeValidate(requireHandle());
  }

  public Result solve(Options options) {
    return Solver.solve(this, options);
  }

  @Override
  protected void release(long handle) {
    nativeDestroy(handle);
  }

  private static native long nativeCreate();
  private static native void nativeDestroy(long handle);
  private static native int nativeAddNode(long handle);
  private static native void nativeSetDistance(long handle, int from, int to, long distance);
  private static native void nativeValidate(long handle);
}
