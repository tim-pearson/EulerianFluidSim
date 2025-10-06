Got it! Let’s break down your **current fluid solver** and math, clearly, concisely, focusing on **physics, methods, and math**, then explain **multi-fluid / buoyancy / inflow**.

---

## **1. Methods and Physics Principles**

Your simulation is a **grid-based incompressible fluid solver** using a **MAC (Marker-and-Cell) grid**:

1. **Advection**:

   * Semi-Lagrangian scheme (`advect`)
   * Backtrace particles/cells along velocity:
$$
     \mathbf{x}_{t+\Delta t} = \mathbf{x}_t - \Delta t , \mathbf{u} \mathbf{x}_t)
$$
   * Ensures **mass/momentum transport**.
   * Uses bilinear interpolation of velocity for arbitrary points.

2. **Pressure Solve (Incompressibility)**:

   * **Poisson equation** for pressure (p) to enforce divergence-free velocity:
$$
     \nabla^2 p = \nabla \cdot \mathbf{u}^*
$$
   * Iterative **Jacobi solver** to approximate solution.
   * Then subtract gradient:
$$
     \mathbf{u}^{n+1} = \mathbf{u}^* - \nabla p
$$
   * This enforces **mass conservation** ($nabla \cdot \mathbf{u} = 0)).

3. **Divergence computation**:
$$
\text{div}(u,v) = \frac{\partial u}{\partial x} + \frac{\partial v}{\partial y}
$$

   * Computes residual divergence for pressure solve.

4. **VOF / Scalar field advection** (`advect_vof`):

   * **Volume-of-Fluid (VOF)** method for fluid fraction (f \in [0,1])
   * Backtrace + scatter ensures **mass conservation** for fluid fraction.
   * Clamping and redistribution handle **overflow / underflow**.

---

## **2. Math Summary**

* **Advection (semi-Lagrangian)**:
$$
\mathbf{u}_{i,j}^{n+1} = \mathbf{u}( \mathbf{x}_{i,j} - \Delta t , \mathbf{u} (\mathbf{x}_{i,j}))
$$

* **Pressure solve (Jacobi iteration)**:
$$
  p_{i,j}^{k+1} = \frac{1}{4} (p_{i-1,j}^k + p_{i+1,j}^k + p_{i,j-1}^k + p_{i,j+1}^k - \text{div}_{i,j})
$$

* **Subtract pressure gradient**:
$$
  u_{i,j}^{n+1} = u_{i,j}^{*} - (p_{i,j} - p_{i,j-1})
$$
$$
  v_{i,j}^{n+1} = v_{i,j}^{*} - (p_{i,j} - p_{i-1,j})
$$

* **VOF mass redistribution**:
$$
  f_{i,j}^{n+1} = \sum_{k,l \in \text{neighbor}} f_{k,l}^{n} , w_{kl\rightarrow ij}
$$
  with weights (w) from bilinear interpolation.

---

## **3. Multi-fluid / Buoyancy**

### **Buoyancy Force (simplest start)**

* For **multi-fluid density differences**, define density $rho(x,y)) per cell:
$$
\mathbf{f}_b = - g (\rho - \rho_0) \hat{y}
$$
* Implementation:

```cpp
v(j,i) += -gravity * (rho(j,i) - rho0) * deltaTime;
```

* This allows **interaction between fluids**, e.g., lighter fluid rises.

### **Handling Multiple Fluids**

1. **Store density / color / VOF per fluid**:

   ```cpp
   Kokkos::View<float**> f1, f2; // fractions for 2 fluids
   ```
2. **Advect each fluid separately** using `advect_vof`.
3. **Mixing**: when sum of fractions > 1, normalize:
$$
   f_1 + f_2 \le 1
$$
4. **Compute weighted density** for pressure / buoyancy:
$$
   \rho_{cell} = f_1 \rho_1 + f_2 \rho_2
$$

---

## **4. Adding Fluid (Inflow / Source)**

* Specify **source region** and **velocity / fraction**:

```cpp
for(int j=y0; j<y1; j++)
  for(int i=x0; i<x1; i++){
    f(j,i) = 1.0f;          // full fluid
    u(j,i) = u_inflow;      // set inflow velocity
    v(j,i) = v_inflow;
  }
```

* Works with **VOF advection**; backtrace will move fluid naturally.

---

## **5. Mixing Cells / Simplest Setup**

* Single VOF scalar per cell: fraction (f)
* Multi-fluid: array of fractions ([f_1,f_2,...])
* Example: **two fluids** (water, oil)
$$
  f_1 + f_2 \le 1, \quad \rho_{cell} = f_1 \rho_1 + f_2 \rho_2
$$
* Initial setup:

```text
f1 = 1 in bottom half
f2 = 1 in top half
rho1 = 1.0, rho2 = 0.8
```

* Apply **buoyancy**, advect velocities and VOF → fluids rise and mix.


