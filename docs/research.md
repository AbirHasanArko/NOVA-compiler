# Research Specifications: Empirical Studies & Bug Hunting

**Status:** Empirical Research Charter (Data-Driven Publication Track)  
**Target Phase:** Phases 15–17  
**Selected Track:** Track A (The Bug Hunter - Differential Fuzzing)

---

## 1. The Critical Truth About Compiler Research

After deep, exhaustive searches across modern programming language literature (E-graphs, SMT Solvers like Dafny/Liquid Types, Csmith/Alive2, and Backward Data-Flow), the conclusion is absolute: 
**There is no theoretical algorithmic novelty left to invent in this space for a project of this scale.** 

If we write a paper claiming we "invented" a new fuzzing architecture or a new SMT optimization pass, it will be instantly desk-rejected by any prestigious journal for lacking novelty.

---

## 2. The Path to Publication: Empirical Results

Since we cannot invent a novel algorithm, we have formally shifted from *Theoretical Computer Science* to *Empirical Software Engineering*.

The publication will not be about the NOVA compiler's architecture. The compiler is merely a tool used to generate **novel data**. Prestigious venues (like ICSE, ISSTA, or EMSE) publish papers that provide the community with new, measured insights or discover critical flaws in existing systems.

### Official Selection: Track A (The Bug Hunter)
The NOVA project has officially adopted the Differential Fuzzing track to find zero-day miscompilations in production compilers.

- **The Methodology:** We will build a domain-specific fuzzer within NOVA that generates massive, contract-heavy ASTs (utilizing `verify`, `require`, and `flux` mutations). NOVA will cross-compile these ASTs into C++ code heavily laden with `[[assume]]` and `assert` statements, feeding them to the latest builds of LLVM (Clang) and GCC.
- **The Oracle:** We will differentially execute the heavily-optimized LLVM/GCC binaries against the unoptimized NOVA AST Interpreter.
- **The Publishable Contribution:** The publication relies **100% on the results**. If we find 10+ zero-day miscompilation bugs in LLVM's or GCC's handling of complex assumptions, we publish a paper detailing the *nature of those bugs*, categorizing them, and explaining why production optimizers still struggle with contract-guided data-flow. 

---

## 3. Immediate Roadmap Alignment

To achieve this bleeding-edge empirical research, we must have a flawless AST generator. The fuzzer cannot generate programs until the Syntax Grammar and AST node hierarchy are perfectly mapped. 

Therefore, our immediate, non-negotiable priority is returning to the core compiler pipeline:
1. **Phase 3:** Complete the Bison Parser.
2. **Phase 4:** Construct the polymorphic AST subsystem.
