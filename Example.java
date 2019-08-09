import java.security.SecureRandom;

public class Example {

  public static void main(String[] args) throws Exception {

    final SecureRandom sr = SecureRandom.getInstance(args[0]);

    for (int i = 0; i < 1000000; i++) {

      sr.generateSeed(8);

    }

  }

}
