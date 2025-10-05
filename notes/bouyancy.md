### **1. What buoyancy is**

Buoyancy is the **force that makes less dense fluids rise and more dense fluids
sink**. It comes from gravity acting differently on fluids of different
densities.

In a discrete CFD cell, you can write it as a **body force** added to the
vertical velocity:

$$
F_y = g \cdot (\rho_\text{cell} - \rho_\text{ref})
$$


Where:

* `g` = gravitational acceleration (positive downwards, usually)
* `rho_cell` = actual density of the fluid in the cell
* `rho_ref` = **reference density** for the medium (or the “background” fluid)

---

### **2. How the reference density works**

* `rho_ref` is **the density you consider “neutral”**:

  * If `rho_cell = rho_ref` → no buoyancy → the fluid neither rises nor sinks.
  * If `rho_cell < rho_ref` → fluid is lighter → buoyancy pushes it up.
  * If `rho_cell > rho_ref` → fluid is heavier → buoyancy pulls it down.

* It’s usually constant across the domain, e.g., `rho_ref = 1.0` for water.

---

### **3. Where to apply it**

In your simulation:

1. Compute **cell mixture density**:

```cpp
float rho_cell = f1 * rho1 + f2 * rho2 + ...;
```

2. Compute buoyancy acceleration:

```cpp
float a_buoy = gravity * (rho_cell - rho_ref);
```

3. Apply to **y-velocity before pressure projection**:

```cpp
v(j,i) += a_buoy * deltaTime;
```

---

### **4. Notes / pitfalls**

* Buoyancy only **affects velocity**, not the fractions directly.
* After adding buoyancy, you must **still enforce incompressibility** via your
pressure projection step. Otherwise the simulation can blow up.
* If you eventually want **large density contrasts**, you may need
**variable-density Navier-Stokes** instead of assuming constant density.

---

In short:

* `rho_ref` = density of the background / neutral fluid.
* `rho_cell` = the density at the current cell (weighted by fraction of each
fluid).
* `a_buoy = g * (rho_cell - rho_ref)` is the acceleration due to buoyancy.

---

If you want, I can **rewrite your `advect()` / `advect_vof()` snippet** to
include **multi-fluid buoyancy** so you can drop it directly in your Kokkos
solver.

Do you want me to do that?
