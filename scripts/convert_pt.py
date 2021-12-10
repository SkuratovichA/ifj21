##
# Convert precedence table to precedence functions.
# @author Evgeny Torbin <xtorbi00@vutbr.cz>
##

# Groups of operators
op_groups = {
	"A": ["MUL", "DIV_I", "DIV_F", "PERCENT"],
	"B": ["ADD", "SUB"],
	"C": ["LT", "LE", "GT", "GE", "EQ", "NE"],
	"D": ["HASH", "NOT", "MINUS_UNARY"]
}

# Precedence table labels
p_table_labels = [
	"ID",
	"CARET",
	"A",
	"B",
	"C",
	"D",
	"CONCAT",
	"AND",
	"OR",
	"DOLLAR"
]

# Precedence table
# A = {*, /, //, %}
# B = {+, -}
# C = {<, <=, >, >=, ==. ~=}
# D = {#, not, - (unary)}
p_table_data = [
    # id   ^    A    B    C    D    ..  and   or   $
	["x", ">", ">", ">", ">", ">", ">", ">", ">", ">"], # id
	["<", ">", ">", ">", ">", ">", ">", ">", ">", ">"], # ^
	["<", "<", ">", ">", ">", "<", ">", ">", ">", ">"], # A
	["<", "<", "<", ">", ">", "<", ">", ">", ">", ">"], # B
	["<", "<", "<", "<", ">", "<", "<", ">", ">", ">"], # C
	["<", "<", ">", ">", ">", "<", ">", ">", ">", ">"], # D
	["<", "<", "<", "<", ">", "<", "<", ">", ">", ">"], # ..
	["<", "<", "<", "<", "<", "<", "<", ">", ">", ">"], # and
	["<", "<", "<", "<", "<", "<", "<", "<", ">", ">"], # or
	["<", "<", "<", "<", "<", "<", "<", "<", "<",  ""], # $
]

def print_longest_paths(paths):
	print("Longest paths:")
	for v in paths:
		print(v + " = " + str(paths[v]))
			
def print_dict (graph):
	print("graph = {")
	for key, value in graph.items():
		if not value:
			print("\n\t" + key + ": [],\n")
			continue
	
		print("\n\t" + key + ": [")
		for item in value:
			print("\t\t" + item + ",")
		print("\t],\n")
	print("}")
	
def result_expand (paths, op_groups):	
	op_groups_ext = {}
	
	for op, arr in op_groups.items():
		op_groups_ext["f_" + op] = []
		op_groups_ext["g_" + op] = []
		for i in arr:
			op_groups_ext["f_" + op].append("f_" + i)
			op_groups_ext["g_" + op].append("g_" + i)
	
	for v in op_groups_ext:
		if v in paths:
			for i in op_groups_ext[v]:
				paths[i] = paths[v]
			del paths[v]
	
	return paths
	
def print_sort (paths, p_table_labels, op_groups):
	f_output = []
	g_output = []
	
	for label in p_table_labels:
		if label in op_groups:
			for op in op_groups[label]:
				f_output.append(paths["f_" + op])
				g_output.append(paths["g_" + op])
		else:
			f_output.append(paths["f_" + label])
			g_output.append(paths["g_" + label])
	
	print("f =", f_output, end = "\n")
	print("g =", g_output, end = "\n")

##
# @brief Convert precedence table to the graph
# of precedence functions.
#
# @param table Precedence table.
# @param labels Labels of the precedence table.
#
# @return dictionary { vertex_name : vertexes_list }.
#
def table_to_graph(table, labels):
	new_graph = {}
	table_size = len(labels)
	
	for row in range(table_size):
		row_label = "f_" + labels[row]
		new_graph[row_label] = []
		
		for col in range(table_size):
			col_label = "g_" + labels[col]
			if row == 0:
				new_graph[col_label] = []
				
			if table[row][col] == "<":
				new_graph[col_label].append(row_label)
			elif table[row][col] == ">":
				new_graph[row_label].append(col_label)
	
	return new_graph

##
# @brief Find the longest path.
#
# @param graph Initial graph.
# @param vertex Start vertex.
# @param debug Print steps.
# @param dist_max Maximal distance (default = 0)
# @param dist Actual distance (default = 0).
#
# @return int.
#
def find_path(graph, vertex, debug, dist_max = 0, dist = 0):
    # Create indents for every level of the graph to debug print
    tab = "\t" * dist if debug else ""
 
    # If we are at the end or vertex does not have any connections   
    if vertex == "f_DOLLAR" or vertex == "g_DOLLAR" or not graph[vertex]:
        if debug:
            print(tab + "[PATH] = " + str(dist))
        
        # If actual distance is greater than maximal, return it
        return dist if dist > dist_max else dist_max
    
    # Increment actual distance
    dist += 1
    
    for v in graph[vertex]:
        if debug:
            print(tab + v)
            
        # Recursive call in order to find all distances
        dist_max = find_path(graph, v, debug, dist_max, dist)
        
    return dist_max
  
##
# @brief Find longest paths.
# 
# @param graph Initial graph. 
# @param debug Print steps.
# @param op_gropus Groups of operators.
#
# @return {}.
#
def find_longest_paths(graph, debug, op_groups):
    longest_paths = {}

    if debug:
        print("<<< DEBUG_PRINT >>>\n")

    for v in graph:
        if debug:
            print("-------------")
            print(v)
            print("-------------\n")
            
        # Add new vertex with path value.
        longest_paths[v] = find_path(graph, v, debug)
        
        if debug:
            print()

    if debug:
	    print("<<< DEBUG_PRINT >>>\n")

    longest_paths = result_expand(longest_paths, op_groups)
    return longest_paths

##
# @brief Main part.
#
    
# Create a precedence graph from precedence table
p_graph = table_to_graph(p_table_data, p_table_labels)

# Print result graph
#print_dict(p_graph)

# Find longest paths of each vertex to $
paths = find_longest_paths(p_graph, False, op_groups)

# Print longest paths in format VERTEX: PATH
#print_longest_paths(paths)

# Sorted print of the longest paths of each vertex
print_sort(paths, p_table_labels, op_groups)
