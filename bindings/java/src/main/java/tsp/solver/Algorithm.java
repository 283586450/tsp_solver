package tsp.solver;

public enum Algorithm {
  DEFAULT(0),
  LOCAL_SEARCH_2OPT(1);

  private final int nativeValue;

  Algorithm(int nativeValue) {
    this.nativeValue = nativeValue;
  }

  int nativeValue() {
    return nativeValue;
  }
}
