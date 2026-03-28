package tsp.solver;

abstract class NativeResource implements AutoCloseable {
  protected long nativeHandle;

  protected final long requireHandle() {
    if (nativeHandle == 0) {
      throw new IllegalStateException(getClass().getSimpleName() + " is closed");
    }
    return nativeHandle;
  }

  @Override
  public final void close() {
    long handle = nativeHandle;
    if (handle == 0) {
      return;
    }

    nativeHandle = 0;
    release(handle);
  }

  protected abstract void release(long handle);
}
