package tsp.solver;

public enum Status {
  NOT_SOLVED(0),
  FEASIBLE(1),
  OPTIMAL(2),
  INFEASIBLE(3),
  TIME_LIMIT(4),
  INVALID_MODEL(5),
  INTERNAL_ERROR(6);

  private final int nativeValue;

  Status(int nativeValue) {
    this.nativeValue = nativeValue;
  }

  static Status fromNative(int nativeValue) {
    for (Status status : values()) {
      if (status.nativeValue == nativeValue) {
        return status;
      }
    }
    throw new SolverNativeException("Unknown native status: " + nativeValue);
  }
}
