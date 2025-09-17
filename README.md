# Fluid Simulation with MAC Grid

This project implements a 2D incompressible and mass-conserving Eulerian Fluid
Simulator using a **MAC (Marker-and-Cell) grid**.

### Tools/Frameworks
- **Kokkos** – parallel simulation steps  
- **OpenGL** – rendering the grid  
- **ImGui** – user interface  

> Note: To enforce true mass conservation, the **pressure projection** method
should be used rather than shortcuts like clearing divergence.

# Implementation

We consider a **MAC grid** of size $(n+2) \times (n+2)$ to account for halo
cells. The main fields are:

- $u$: x-velocity, stored at horizontal faces $(n, n+1)$  
- $v$: y-velocity, stored at vertical faces $(n+1, n)$  
- $p$: pressure, stored at cell centers $(n, n)$  
- $s$: solid indicator (1 = fluid, 0 = solid)  
- $d$: divergence, computed per cell  

---

## 1. Compute Divergence

For each fluid cell $(i,j)$:

$$
d_{i,j} = (u_{i,j+1} - u_{i,j}) + (v_{i+1,j} - v_{i,j})
$$

This measures net flow out of the cell.

---

## 2. Clear Divergence (Red-Black Gauss–Seidel)

> **Shortcut:** This method reduces divergence but does **not guarantee mass
conservation**. It’s mainly for quick divergence reduction in simpler setups.

Iterate over cells, skipping solid neighbors:

$$
\text{residual } r = d_{i,j} \cdot \omega
$$

Neighbor weights from the $s$ grid:

$$
s_L = s_{i,j-1}, \quad s_R = s_{i,j+1}, \quad s_D = s_{i-1,j}, \quad s_U =
s_{i+1,j}
$$

Update velocities:

$$
\begin{aligned}
u_{i,j} &\mathrel{+}= r \frac{s_L}{s_L+s_R+s_D+s_U}, \\
u_{i,j+1} &\mathrel{-}= r \frac{s_R}{s_L+s_R+s_D+s_U}, \\
v_{i,j} &\mathrel{+}= r \frac{s_D}{s_L+s_R+s_D+s_U}, \\
v_{i+1,j} &\mathrel{-}= r \frac{s_U}{s_L+s_R+s_D+s_U}.
\end{aligned}
$$

---

## 3. Solve Pressure (Jacobi Iteration)

This step enforces true incompressibility and mass conservation:

$$
p_{i,j}^{\text{new}} = \frac{1}{4} \Big( p_{i-1,j} + p_{i+1,j} + p_{i,j-1} +
p_{i,j+1} - d_{i,j} \Big)
$$

Swap temporary and main pressure fields after each iteration.

---

## 4. Subtract Pressure Gradient

Update velocities to enforce divergence-free flow:

$$
\begin{aligned}
u_{i,j} &\mathrel{-}= p_{i,j} - p_{i,j-1}, \\
v_{i,j} &\mathrel{-}= p_{i,j} - p_{i-1,j}.
\end{aligned}
$$

> This is the preferred way to enforce mass conservation rather than clearing
divergence.

---

## 5. Advect Volume Fraction (VOF)

For each fluid cell:

1. Backtrace particle position using velocity:

$$
\begin{aligned}
x_{\text{prev}} &= x - u \Delta t, \\
y_{\text{prev}} &= y - v \Delta t
\end{aligned}
$$

2. Compute bilinear weights:

$$
\begin{aligned}
w_{00} &= (1-\alpha)(1-\beta), \quad w_{01} = (1-\alpha)\beta, \\
w_{10} &= \alpha(1-\beta), \quad w_{11} = \alpha \beta
\end{aligned}
$$

3. Scatter mass back to the previous grid cells:

$$
f_{i,j}^{\text{new}} = \sum_{m,n \in \{0,1\}} w_{mn} f_{i-m,j-n}^{\text{old}}
$$

4. Clamp to $[0,1]$ and redistribute overflow to neighboring fluid cells to
maintain conservation.