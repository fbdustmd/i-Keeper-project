# NetSentry Execution Plan Rules

## 1. When a Plan Is Required

Create an execution plan for work that:
- touches multiple modules
- changes data structures shared across modules
- introduces a new pipeline stage
- changes TCP Flow behavior
- changes reassembly behavior
- requires several tests

A tiny typo or isolated logging change does not need a full plan.

## 2. Location

Active plans:

```text
docs/exec-plans/active/
```

Completed plans:

```text
docs/exec-plans/completed/
```

Use names like:

```text
phase-06-flow-manager.md
phase-07-basic-reassembly.md
bug-truncated-ipv4.md
```

## 3. Plan Template

Historical or multi-phase reference drafts live in `docs/exec-plans/reference/`.
They are not current execution plans; use ROADMAP.md for phase numbering.

Each plan should contain:

```markdown
# Title

## Goal

What exact behavior should exist when this plan is complete?

## Non-goals

What related work is intentionally excluded?

## Current State

Which relevant files/functions already exist?

## Design

What structures/functions will be introduced or changed?

## Steps

- [ ] Step 1
- [ ] Step 2
- [ ] Step 3

## Validation

Which commands/tests prove the work is correct?

## Risks

What can easily go wrong?

## Completion Notes

What actually changed?
What limitations remain?
```

## 4. Plan Discipline

Plans are working documents.

During implementation:
- check off completed steps
- record design changes
- record discovered limitations
- do not pretend the original plan stayed correct if implementation changed

## 5. Small Steps

Prefer a sequence like:

```text
Flow key structure
→ flow lookup
→ reverse-direction matching
→ direction classification
→ unit tests
```

over:

```text
Implement complete Flow Manager
```

Each step should leave the repository buildable whenever practical.

## 6. Reassembly Planning

Never create a plan titled only:

```text
Implement TCP reassembly
```

Split it:

1. segment representation
2. in-order append
3. out-of-order storage
4. contiguous flush
5. exact duplicate handling
6. simple retransmission handling

More advanced overlap behavior belongs to a later plan.
