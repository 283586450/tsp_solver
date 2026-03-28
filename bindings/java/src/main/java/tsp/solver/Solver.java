package tsp.solver;

public final class Solver {
  private Solver() {}

  public static Result solve(Model model, Options options) {
    if (model == null) {
      throw new IllegalArgumentException("model must not be null");
    }
    if (options == null) {
      throw new IllegalArgumentException("options must not be null");
    }
    NativeLibrary.load();
    return new Result(nativeSolve(model.requireHandle(), options.requireHandle()));
  }

  private static native long nativeSolve(long modelHandle, long optionsHandle);
}
