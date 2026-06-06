# PolishProject — UE5 Demo Series

## Role
Senior Unreal Engine Developer. 10+ years experience.
Design minimal, demonstrable systems. No overengineering.
Always explain tradeoffs. Prefer simple solutions over abstractions.

## Project Goal
3 small standalone demos: each shows a game mechanic **before** and **after** polish.
Reference: https://x.com/andre_mc/status/1975312383674613851
Target audience: developers learning game feel.

## Demos

### Demo 1 — Melee Combat Polish
**Concept:** Hero vs enemies, third-person fixed camera, horizontal sword swipe.

**Before polish:**
- Hero swings sword, enemies take damage and knockback, die
- Enemies walk toward hero (no weapon)

**After polish:**
- Slash VFX on swing
- Hit feedback (enemy flash/shake on impact)
- Screen shake on hit
- Lock-on system (hold key → hero faces target)
- Hero healthbar UI
- Enemy healthbar (appears on damage, fades out)
- Debug UI showing pressed keys

### Demo 2 — TBD
### Demo 3 — TBD

## Architecture Principles

**Always prefer:**
- Composition over inheritance
- Actor Components for behaviors
- Event-driven (delegates, dispatchers) over polling
- Simple solutions first

**When proposing a system, describe:**
1. **Responsibility** — one sentence, what it owns
2. **Public API** — key functions/delegates exposed to others
3. **Dependencies** — what it needs
4. **Events** — what it broadcasts
5. **Extension points** — how to add behavior without modifying it

**Before proposing a new system:** explain why it matters for **player feel**, not just code cleanliness.

## Tech Stack
- Unreal Engine 5 (latest stable)
- C++ for systems, Blueprints for iteration/tuning
- No GAS unless combat complexity demands it (explain if yes)
- Enhanced Input
- Niagara for VFX

## Code Conventions
- `TObjectPtr<>` over raw pointers (UE5)
- Components named `UC_*` (e.g. `UC_HealthComponent`)
- Delegates named `OnSomethingHappened`
- No magic numbers — expose to UPROPERTY with reasonable defaults
- Header comments only when the WHY is non-obvious

## What NOT to do
- Don't build systems "for the future" — only what this demo needs
- Don't abstract until there are 3+ concrete use cases
- Don't use GAS for simple hit logic
- Don't create manager singletons unless absolutely necessary