## Batching methods
- Batch packets to amortize instruction costs AND to increase 
  locality(?)

## Prefetching instructions
- Prefetch whatever cache-misses then (try to) reduce the 
  data-dependency between the data that is being prefetched and the 
  operation on the prefetched data.
  
## Replacing a branch with a no-branch counterpart 
- This one is difficult: instead of doing:
  if (blah blah) { do XX; } else { do YY; }
  we can do: (do XX and do YY) and then decide 
  which one to record.

## SIMD instruction templates for packet processing 
- No ideas ATM.  Maybe look at Yang's paper on the topic?

## Transformations for reducing data-dependency 
- If two consecutive instructions have data-dependency breakdown the 
  flow by adding more packets in between the data-dependencies to 
  increase locality.  (How to detect?)
  

## Fast path extraction through forward-slicing of the NF code
- Look for how the code for a specific module is getting executed.  
  Do static symbolic computation on that path.  Use that slice instead 
  to process that packet.
  
  Need to know whether the operations are also commutative.  Or at least 
  have some form of relaxation criteria.

## Data structure tuning
- Merging rows to increase locality?
- Data structure type?

## Flow consistency guarantees
We should have some guarantees on the ordering of the flows:
e.g., packets are ordered with respect to specific bytes.

E.g., pkt[ip.src, ip.dst, port.src, port.dst, protocol] consistency.

This means that we are allowed to KEEP or REORDER for pkt[X]:

bowtie = commutes

\begin{align*}
&\text{Can we reorder a program that guarantees consistency of}\;p[Y]\;\text{in order to guarantee consistency of}\;p[X]\text{?}\\
&X \subset Y: Y1 \not= Y2 \Rightarrow p[Y1] \not\parallel p[Y2]\;\text{under mostt conditions.} \\
&Y \subset X: Y1 \not= Y2 \Rightarrow p[Y1] \parallel p[Y2]\;\text{under any conditions.  }
\end{align*}

