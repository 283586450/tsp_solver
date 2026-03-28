package tsp.solver;

import java.util.Arrays;

public final class BindingTestMain {
  private BindingTestMain() {}

  public static void main(String[] args) {
    testVersionMismatchMessage();
    testSolveCompleteGraph();
    testValidationError();
    testRangeError();
    testCloseIsIdempotent();
  }

  private static void testSolveCompleteGraph() {
    try (Model model = new Model(); Options options = new Options()) {
      options.setAlgorithm(Algorithm.LOCAL_SEARCH_2OPT);

      for (int i = 0; i < 4; ++i) {
        assertEquals(i, model.addNode(), "node id");
      }

      long[][] distances = {
          {0, 2, 9, 10},
          {1, 0, 6, 4},
          {15, 7, 0, 8},
          {6, 3, 12, 0},
      };

      for (int from = 0; from < distances.length; ++from) {
        for (int to = 0; to < distances[from].length; ++to) {
          model.setDistance(from, to, distances[from][to]);
        }
      }

      model.validate();

      try (Result result = model.solve(options)) {
        assertEquals(Status.FEASIBLE, result.getStatus(), "status");
        assertEquals(4, result.getTourSize(), "tour size");
        int[] tour = result.getTour();
        Arrays.sort(tour);
        assertTrue(Arrays.equals(tour, new int[] {0, 1, 2, 3}), "tour permutation");
        assertTrue(result.getObjective() >= 0, "objective should be non-negative");
      }
    }
  }

  private static void testValidationError() {
    try (Model model = new Model()) {
      model.addNode();
      model.addNode();

      expectThrows(IllegalStateException.class, new ThrowingRunnable() {
        @Override
        public void run() {
          model.validate();
        }
      }, "incomplete model must fail validation");
    }
  }

  private static void testRangeError() {
    try (Model model = new Model()) {
      model.addNode();

      expectThrows(IndexOutOfBoundsException.class, new ThrowingRunnable() {
        @Override
        public void run() {
          model.setDistance(1, 0, 5);
        }
      }, "out of range node id must fail");
    }
  }

  private static void testCloseIsIdempotent() {
    Model model = new Model();
    model.close();
    model.close();

    expectThrows(IllegalStateException.class, new ThrowingRunnable() {
      @Override
      public void run() {
        model.addNode();
      }
    }, "closed model must reject further use");
  }

  private static void testVersionMismatchMessage() {
    expectThrowsMessage(IllegalStateException.class,
        "Java binding version 9.9.9 does not match native library version 0.1.0. Use the jar and native bundle from the same tsp_solver release.",
        new ThrowingRunnable() {
      @Override
      public void run() {
        NativeLibrary.load("9.9.9");
      }
    }, "mismatched Java and native versions must fail");
  }

  private static void assertTrue(boolean condition, String message) {
    if (!condition) {
      throw new AssertionError(message);
    }
  }

  private static void assertEquals(Object expected, Object actual, String message) {
    if (!expected.equals(actual)) {
      throw new AssertionError(message + ": expected=" + expected + " actual=" + actual);
    }
  }

  private static void expectThrows(
      Class<? extends Throwable> type, ThrowingRunnable runnable, String message) {
    expectThrowsMessage(type, null, runnable, message);
  }

  private static void expectThrowsMessage(
      Class<? extends Throwable> type,
      String expectedMessage,
      ThrowingRunnable runnable,
      String message) {
    try {
      runnable.run();
    } catch (Throwable error) {
      if (type.isInstance(error)) {
        if (expectedMessage != null && !expectedMessage.equals(error.getMessage())) {
          throw new AssertionError(
              message + ": wrong message " + error.getMessage(), error);
        }
        return;
      }
      throw new AssertionError(message + ": wrong exception " + error, error);
    }
    throw new AssertionError(message + ": no exception thrown");
  }

  private interface ThrowingRunnable {
    void run();
  }
}
