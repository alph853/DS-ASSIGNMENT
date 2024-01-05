# This is just the core for algorithms 
This is just source code folder that implements **YENKSP** and **Dinic** Algorithm in "*nodeManager*" class and outputs data to the terminal. This folder does not contain any UI implementaiion. 
- For YENKSP:
  
+ "*dijkstra(...)*" return the shortest path, and the "*topk_shortestPaths(...)*" return the top k shortest paths (in this case is 5).

- For maximum flow: 

+ *levelBFS(...)* checks if there is a path from source to end (the end node can be assigned a specific level), *sendFlowDFS(...)* returns the path and their associated capacity. Lastly, *dinicFlow(...)* returns maximum flow, paths and also capacities of all paths.

- Output path: *printPath(...)* and *printFlow(...)* returns strings that describe the paths taken.

# To run this code
First, please open this folder in file explorer and open this in terminal. Then, to run this .exe file, run the following command to see road direction.
## run command: 
- 3 arguments: "mode" "source" "end" 
- mode:
+ ksp: k shortest paths
+ maxflow: maximum flow

example: "**./main ksp 1 140**": the mode chosen is top k shortest paths with source 1 and end 140.





