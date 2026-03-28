import java.util.Arrays;

import tsp.solver.Algorithm;
import tsp.solver.Model;
import tsp.solver.Options;
import tsp.solver.Result;
import tsp.solver.Status;

public final class PackageSmokeMain {
  private PackageSmokeMain() {}

  public static void main(String[] args) {
    try (Model model = new Model(); Options options = new Options()) {
      options.setAlgorithm(Algorithm.LOCAL_SEARCH_2OPT);

      for (int i = 0; i < 4; ++i) {
        model.addNode();
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
        if (result.getStatus() != Status.FEASIBLE) {
          throw new AssertionError("expected FEASIBLE status");
        }
        if (result.getAlgorithm() != Algorithm.LOCAL_SEARCH_2OPT) {
          throw new AssertionError("expected 2-opt algorithm");
        }
        if (result.getTourSize() != 4) {
          throw new AssertionError("expected 4-node tour");
        }

        int[] tour = result.getTour();
        Arrays.sort(tour);
        if (!Arrays.equals(tour, new int[] {0, 1, 2, 3})) {
          throw new AssertionError("expected tour permutation");
        }
      }

      options.setAlgorithm(Algorithm.GREEDY_NEAREST_NEIGHBOR);
      options.setRandomSeed(42);
      long seedObjective;
      try (Result result = model.solve(options)) {
        seedObjective = result.getObjective();
      }

      options.setAlgorithm(Algorithm.METAHEURISTIC_ITERATED_LOCAL_SEARCH);
      options.setRandomSeed(42);
      try (Result result = model.solve(options)) {
        if (result.getStatus() != Status.FEASIBLE) {
          throw new AssertionError("expected FEASIBLE status");
        }
        if (result.getAlgorithm() != Algorithm.METAHEURISTIC_ITERATED_LOCAL_SEARCH) {
          throw new AssertionError("expected iterated local search algorithm");
        }
        if (result.getTourSize() != 4) {
          throw new AssertionError("expected 4-node tour");
        }
        if (result.getObjective() > seedObjective) {
          throw new AssertionError("expected objective to not exceed seed");
        }
      }

      options.setAlgorithm(Algorithm.HELD_KARP);
      try (Result result = model.solve(options)) {
        if (result.getStatus() != Status.OPTIMAL) {
          throw new AssertionError("expected OPTIMAL status");
        }
        if (result.getAlgorithm() != Algorithm.HELD_KARP) {
          throw new AssertionError("expected Held-Karp algorithm");
        }
        if (result.getTourSize() != 4) {
          throw new AssertionError("expected 4-node tour");
        }
        if (result.getObjective() != 21L) {
          throw new AssertionError("expected optimal objective");
        }
      }
    }
  }
}
