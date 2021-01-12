import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class Main {
    private static Graph graph;

    public static void readGraph() throws FileNotFoundException {
        File inputFile=new File("C:\\Users\\Asus\\OneDrive\\Documents\\facultate\\sem5\\PDP\\project\\threading\\solution\\src\\main\\resources\\input.in");
        Scanner scanner=new Scanner(inputFile);

        int numberOfVertices=Integer.parseInt(scanner.nextLine());

        graph=new Graph(numberOfVertices);

        while(scanner.hasNextLine())
        {
            String line=scanner.nextLine();
            int v1=Integer.parseInt(line.split(" ")[0]);
            int v2=Integer.parseInt(line.split(" ")[1]);
            graph.addEdge(v1,v2);
        }

        scanner.close();

    }

    public static void main(String[] args) {
        System.out.println("Find an n-coloring of a graph.\n");

        //initialize the graph
        try {
            readGraph();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        long startTime=System.nanoTime();

        graph.colorGraph();

        long endTime=System.nanoTime();

        long time=endTime-startTime;

        graph.printColors();

        System.out.println("Duration: "+time/1000000 + " milliseconds.");

    }

}
