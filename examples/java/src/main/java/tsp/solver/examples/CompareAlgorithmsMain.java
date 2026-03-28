package tsp.solver.examples;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import tsp.solver.Algorithm;
import tsp.solver.Model;
import tsp.solver.Options;
import tsp.solver.Result;

public final class CompareAlgorithmsMain {
  private static final class Matrix {
    final int size;
    final long[][] values;

    Matrix(int size, long[][] values) {
      this.size = size;
      this.values = values;
    }
  }

  private static final class AlgorithmRun {
    final String label;
    final Algorithm algorithm;

    AlgorithmRun(String label, Algorithm algorithm) {
      this.label = label;
      this.algorithm = algorithm;
    }
  }

  private static final List<AlgorithmRun> RUNS = List.of(
      new AlgorithmRun("TSP_SOLVER_ALGORITHM_DEFAULT", Algorithm.DEFAULT),
      new AlgorithmRun("TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR",
          Algorithm.GREEDY_NEAREST_NEIGHBOR),
      new AlgorithmRun("TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT", Algorithm.LOCAL_SEARCH_2OPT));

  private CompareAlgorithmsMain() {}

  public static void main(String[] args) {
    if (args.length != 1) {
      System.err.println("usage: CompareAlgorithmsMain <matrix_path>");
      System.exit(1);
    }

    try {
      Matrix matrix = loadMatrix(Path.of(args[0]));
      try (Model model = buildModel(matrix); Options options = new Options()) {
        System.out.println("algorithm\tobjective\ttour");
        for (AlgorithmRun run : RUNS) {
          options.setAlgorithm(run.algorithm);
          try (Result result = model.solve(options)) {
            System.out.println(run.label + "\t" + result.getObjective() + "\t"
                + joinTour(result.getTour()));
          }
        }
      }
    } catch (Exception exc) {
      System.err.println(exc.getMessage());
      System.exit(1);
    }
  }

  private static Matrix loadMatrix(Path path) throws IOException {
    List<String> lines = Files.readAllLines(path);
    List<String> tokens = new ArrayList<>();
    for (String line : lines) {
      String trimmed = line.trim();
      if (!trimmed.isEmpty()) {
        for (String token : trimmed.split("\\s+")) {
          tokens.add(token);
        }
      }
    }

    if (tokens.isEmpty()) {
      throw new IllegalArgumentException("matrix file is empty: " + path);
    }

    int size = Integer.parseInt(tokens.get(0));
    if (size <= 0) {
      throw new IllegalArgumentException("matrix size must be positive: " + path);
    }
    int expected = 1 + size * size;
    if (tokens.size() != expected) {
      throw new IllegalArgumentException(
          "matrix file must contain " + size + "x" + size + " values, found " + (tokens.size() - 1));
    }

    long[][] values = new long[size][size];
    int index = 1;
    for (int row = 0; row < size; ++row) {
      for (int col = 0; col < size; ++col) {
        values[row][col] = Long.parseLong(tokens.get(index++));
      }
    }
    return new Matrix(size, values);
  }

  private static Model buildModel(Matrix matrix) {
    Model model = new Model();
    try {
      for (int i = 0; i < matrix.size; ++i) {
        model.addNode();
      }
      for (int from = 0; from < matrix.size; ++from) {
        for (int to = 0; to < matrix.size; ++to) {
          model.setDistance(from, to, matrix.values[from][to]);
        }
      }
      model.validate();
      return model;
    } catch (RuntimeException exc) {
      model.close();
      throw exc;
    }
  }

  private static String joinTour(int[] tour) {
    StringBuilder builder = new StringBuilder();
    for (int index = 0; index < tour.length; ++index) {
      if (index > 0) {
        builder.append(' ');
      }
      builder.append(tour[index]);
    }
    return builder.toString();
  }
}
