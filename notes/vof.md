# Advecting Volume Fraction (VOF) in a MAC Grid

This procedure implements a **mass-conserving advection** of a volume fraction
field `f` in a 2D Eulerian grid using the **backward trace (scatter) method**.
We assume a MAC grid with a **halo/obstacle cell layer** of width 1 around the
domain. The halo is used to handle boundaries and solid walls.

---

## 1. Grid Setup

- Let the grid be of size $(n+2) \times (n+2)$, including halo cells.
- Fields stored in the grid:
  - $f_{i,j} \in [0,1]$: volume fraction of fluid in cell $(i,j)$.
  - $u_{i,j}$: x-velocity at horizontal faces.
  - $v_{i,j}$: y-velocity at vertical faces.
  - $s_{i,j} \in \{0,1\}$: solid indicator, $1$ = fluid, $0$ = solid/obstacle.

Halo cells at the domain boundary are treated as walls:
$$
s_{0,j} = s_{n+1,j} = s_{i,0} = s_{i,n+1} = 0
$$

---

## 2. Backward Trace (Scatter)

For each fluid cell $(i,j)$, compute the backtraced position:
$$
x_\text{prev} = x - u(x,y) \Delta t, \quad
y_\text{prev} = y - v(x,y) \Delta t
$$
where $(x,y) = (i+0.5, j+0.5)$ is the cell center and $\Delta t$ is the time
step.  

Clamp positions to the valid domain:
$$
0.0 \le x_\text{prev} \le n, \quad
0.0 \le y_\text{prev} \le n
$$

Convert to cell indices:
$$
i_0 = \lfloor x_\text{prev} - 0.5 \rfloor, \quad
j_0 = \lfloor y_\text{prev} - 0.5 \rfloor
$$
and the neighboring cell:
$$
i_1 = i_0 + 1, \quad j_1 = j_0 + 1
$$

Compute bilinear interpolation weights:
$$
\begin{aligned}
w_{x1} &= x_\text{prev} - (i_0 + 0.5), & w_{x0} &= 1 - w_{x1} \\
w_{y1} &= y_\text{prev} - (j_0 + 0.5), & w_{y0} &= 1 - w_{y1} \\
w_{00} &= w_{x0} w_{y0}, & w_{01} = w_{x0} w_{y1} \\
w_{10} &= w_{x1} w_{y0}, & w_{11} = w_{x1} w_{y1}
\end{aligned}
$$

Scatter the mass from $(i,j)$ to neighbors:
$$
\begin{aligned}
f_\text{new}(i_0,j_0) &+= f_{i,j} \, w_{00} \\
f_\text{new}(i_0,j_1) &+= f_{i,j} \, w_{01} \\
f_\text{new}(i_1,j_0) &+= f_{i,j} \, w_{10} \\
f_\text{new}(i_1,j_1) &+= f_{i,j} \, w_{11}
\end{aligned}
$$

---

## 3. Leftover Redistribution

If the total scattered weight is less than 1 due to solid cells blocking
neighbors:
$$
\beta_{i,j} = w_{00} + w_{01} + w_{10} + w_{11} < 1
$$
Redistribute leftover mass to valid neighbors proportionally to velocity
direction.

---

## 4. Clamping and Symmetric Redistribution

After scattering, clamp values to the valid range:
$$
f_\text{new}(i,j) \in [0,1]
$$

For overflow $(f_\text{new} > 1)$, redistribute the excess to fluid neighbors
along the direction of the velocity:
$$
f_\text{new}(i \pm 1,j) += \text{overflow} \cdot \frac{|u|}{|u|+|v|}, \quad
f_\text{new}(i,j \pm 1) += \text{overflow} \cdot \frac{|v|}{|u|+|v|}
$$
If all neighbors are blocked by solid cells, split equally among accessible
neighbors.

---

## 5. Summary

This procedure ensures:

1. **Mass conservation:** all fluid fractions are redistributed without loss.
2. **Advection along the velocity field:** backtracing accounts for the flow.
3. **Obstacle handling:** halo cells and `s_{i,j}` prevent scattering into
walls.
4. **Clamping:** ensures $f_{i,j} \in [0,1]$.

The result is a conservative, divergence-free volume fraction update suitable
for VOF simulations in Eulerian grids.