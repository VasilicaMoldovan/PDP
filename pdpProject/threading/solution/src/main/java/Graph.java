import java.util.*;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

public class Graph {

    private ExecutorService executorService;

    int vertices;
    int edges;

    Map<Integer, Set<Integer>> graph;

    Set<Integer> colors;

    Map<Integer,Node> verticesInfo;

    Set<Integer> independentSet;

    Random random=new Random();

    public Graph(int numberOfVertices)
    {
        executorService= Executors.newFixedThreadPool(8);

        this.vertices=numberOfVertices;

        graph=new HashMap<>();
        verticesInfo=new HashMap<>();

        for(int i=0;i<numberOfVertices;i++)
        {
            graph.put(i,new HashSet<>());
            verticesInfo.put(i,new Node(-1, random.nextInt(101)));
        }

        colors=new TreeSet<>();
        independentSet =new HashSet<>(graph.keySet());

        for(int i=0;i<100;i++)
        {
            colors.add(i);
        }
    }

    public Integer getColor(int vertex)
    {
        return verticesInfo.get(vertex).color;
    }

    public Integer getValue(int vertex)
    {
        return verticesInfo.get(vertex).random;
    }


    public List<Integer> getNeighbours(int vertex)
    {
        return graph.get(vertex).stream().filter((node) -> {
            return getColor(node)==-1;
        }).collect(Collectors.toList());
    }

    public Set<Integer> getAllNeighbours(int vertex)
    {
        return graph.get(vertex);
    }

    public Set<Integer> getNeighbourColors(int vertex)
    {
        return getAllNeighbours(vertex).stream().filter((node)-> {
            return getColor(node) != -1;
        }).map(node -> getColor(node)).distinct().collect(Collectors.toSet());
    }

    public Integer getSmallestColor(int vertex)
    {
        Set<Integer> neighbourColors=getNeighbourColors(vertex);
        for(Integer color:colors)
        {
            if(!neighbourColors.contains(color))
                return color;
        }
        return 0;
    }

    public Set<Integer> getIndependentSet()
    {
        ArrayList<Future<Boolean>> list = new ArrayList<>();
        Set<Integer> res = new HashSet<>();
        List<Integer> l = new ArrayList<>(independentSet);
        for (Integer v : l) {
            Future<Boolean> f = executorService.submit(() -> checkVertex(v));
            list.add(f);
        }

        for (int i = 0; i < independentSet.size(); i++) {
            try {
                if (list.get(i).get())
                    res.add(l.get(i));
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
        return res;
    }

    public void setColor(int vertex)
    {
        verticesInfo.get(vertex).color=getSmallestColor(vertex);
    }

    public void printColors()
    {
        for(Integer vertex:verticesInfo.keySet())
            System.out.println("Vertex: "+vertex+" color: "+getColor(vertex));
    }

    public boolean checkVertex(int vertex)
    {
        for (Integer n:getNeighbours(vertex))
        {
            if(getValue(n)>getValue(vertex))
                return false;
            else if (getValue(n).equals(getValue(vertex)) && vertex>n)
                return false;
        }
        return true;
    }

    public void addEdge(int source, int destination)
    {
        edges++;
        graph.get(source).add(destination);
        graph.get(destination).add(source);
    }

    public void colorGraph()
    {
        while(!independentSet.isEmpty())
        {
            Set<Integer> set= getIndependentSet();
            for(Integer vertex: set)
            {
                executorService.submit(()-> {
                    setColor(vertex);
                });
            }
            independentSet.removeAll(set);
        }
        executorService.shutdown();
    }



}
