package test;

/**
 *
 * @author Stefan
 */
public class ListCommand extends Command{

    public ListCommand(String word) {
        this.word = word;
        this.type = Command.LIST;
    }
}
