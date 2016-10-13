package test;

/**
 *
 * @author Stefan
 */
public class CountCommand extends Command{

    public CountCommand(String word) {
        this.word = word;
        this.type = Command.COUNT;
    }
}
