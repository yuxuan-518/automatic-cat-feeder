# Mechanical Design and Food-Flow Development

**[Project Overview](../README.md) · [Mechanical Design](mechanical-design.md) · [Motor & Automation](motor-and-automation.md) · [Food Preservation](food-preservation.md)**

## Mechanical work by Shuoqi Yang

Shuoqi Yang led the mechanical engineering portion of the automatic cat feeder. He developed the CAD assembly, planned the dimensions and component relationships, sourced and purchased materials, machined and cut the parts, assembled the structure, and evaluated the mechanism through physical food-flow testing.

The assembly includes the food-storage container, hopper, guide rails, dispensing mechanism, supporting frame, and feeding tray. The complete assembly can be downloaded as [`automatic_cat_feeder_assembly.step`](../cad/automatic_cat_feeder_assembly.step).

## From CAD to Physical Behavior

The first CAD model defined how the parts fit together, but the physical prototype showed that loose granular material could not be evaluated through geometry alone. During testing, cat food repeatedly jammed and leaked near the hopper outlet. The failure was localized near the transition between the hopper and dispensing path rather than at the storage container or feeding tray.

Shuoqi observed how the food accumulated and moved through the structure and identified the outlet geometry as the primary mechanical problem. He reduced the outlet area to **50 cm²** and adjusted the surrounding dispensing path so the mechanism released a smaller and more controlled quantity of food at a time.

The revised structure improved the consistency of the flow. This iteration showed how friction, particle shape, alignment, surface condition, structural deflection, and manufacturing tolerances can cause a fabricated mechanism to behave differently from the CAD model.

## Mechanical Operating Sequence

```mermaid
flowchart LR
    A[Food in storage container] --> B[Hopper]
    B --> C[Revised 50 cm² outlet]
    C --> D[Motor-driven dispensing mechanism]
    D --> E[Guide path]
    E --> F[Feeding tray]
```

## Mechanical Status and Limitations

- The complete mechanical structure was modeled, fabricated, assembled, and tested.
- The outlet and dispensing path were redesigned in response to observed jamming and leakage.
- The higher-torque motor can drive the dispensing mechanism when connected.
- The approximately 5 kg replacement motor is not permanently mounted because its mass could shift the feeder's center of gravity and increase tipping risk.
- A future integrated version would require a wider or heavier base, an alternative motor location, or a lighter drive solution before the motor could safely become part of the main frame.

## Files

- [Complete STEP assembly](../cad/automatic_cat_feeder_assembly.step)
- [Completed feeder demonstration](../assets/videos/physical-feeder-demo.mp4)
- [Motor and automation documentation](motor-and-automation.md)
