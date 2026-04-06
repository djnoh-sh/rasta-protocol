# RV-232 Deferred Queue Capacity Generalization Review

## Scope
- adapter outbound deferred queue implementation
- deferred queue capacity literal removal

## Findings
- current deferred queue behavior remains `outstanding 1 + deferred 2`
- queue storage, enqueue guard, front-drop shift, and init paths now read the same named capacity definition
- this narrows the next deeper-backlog change to a single capacity decision plus affected policy/test updates instead of more literal `2` cleanups

## Decision
- accept current refactor as preparatory closeout for deferred queue capacity generalization groundwork
- keep current configured capacity at `2`
- treat actual deeper-backlog growth as a later policy change, not as part of this refactor
