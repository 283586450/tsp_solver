package tsp.solver;

public enum Algorithm {
  DEFAULT(0),
  LOCAL_SEARCH_2OPT(1),
  GREEDY_NEAREST_NEIGHBOR(2),
  GREEDY_CHEAPEST_INSERTION(3),
  HELD_KARP(4),
  METAHEURISTIC_ITERATED_LOCAL_SEARCH(5);

  private final int nativeValue;

  Algorithm(int nativeValue) {
    this.nativeValue = nativeValue;
  }

  int nativeValue() {
    return nativeValue;
  }

  static Algorithm fromNative(int nativeValue) {
    for (Algorithm algorithm : values()) {
      if (algorithm.nativeValue == nativeValue) {
        return algorithm;
      }
    }
    throw new IllegalArgumentException("unknown algorithm value: " + nativeValue);
  }
}
