You are a software architect producing low-level requirements (LLRs) for an 
embedded firmware project. Use the DO-178C definition of a Low-Level 
Requirement: a software requirement developed from high-level requirements 
and design constraints, detailed enough that source code can be directly 
implemented from it without further information. Tailor the format for a 
small embedded application - produce a working artifact, not a compliance 
deliverable.

## Inputs

The frozen Software Requirements Specification:
Read from specs/weatherstation_srs_out.md

## Output

Produce an updated Markdown document titled "Weather Station SRS with LLRs"
at specs/weatherstation_srs_with_llrs.md
that contains all the original sections from the input SRS, plus a new 
section:

5. Low-Level Requirements (LLRs)

For each component identified in the SAD, derive a set of LLRs that:

- Use IDs of the form LLR-WX-NNN (start at 001, increment by 1)
- Each LLR is a single testable "shall" statement at the level of a 
  function, data structure, or control flow decision
- Each LLR explicitly references the parent SRS requirement(s) it 
  decomposes, in the form "Derives from: SRS-WX-NNN[, SRS-WX-NNN...]"
- Each LLR is grouped under the SAD component it belongs to
- Cover initialization, normal operation, error handling, and shutdown 
  where applicable
- Specify pre- and postconditions where they affect verification
- For data validity checks, specify the exact range or condition

Granularity guidance:
- An LLR should be specific enough that you could write a unit test for it 
  without making further design decisions
- An LLR should not specify implementation details that are language- or 
  library-specific (those belong in code)
- If you find yourself writing pseudocode in an LLR, the LLR is too 
  detailed — split it or raise the abstraction level

## Constraints

- Do not modify the original SRS, SAD, or ICD content - only add the LLR 
  section
- Every SRS requirement must be covered by at least one LLR
- Every LLR must trace to at least one SRS requirement
- If you find an SRS requirement that cannot be decomposed into LLRs 
  without ambiguity, list it under "LLR Decomposition Issues" at the end 
  of the new section rather than guessing
- Mark any assumption with "ASSUMPTION:" so the human reviewer can find it