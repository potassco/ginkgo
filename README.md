# ginkgo—Tools for Generalizing Learned Constraints

`ginkgo` uses ASP techniques to automatically generalize learned conflict constraints to make them applicable to other PDDL problem instances.

`ginkgo` proceeds in a four-step approach:

1. Conflict constraints are extracted while solving a logic program with [`xclasp`](https://github.com/potassco/xclasp), a variant of [`clasp`](https://github.com/potassco/clasp) extended with facilities for logging learned constraints.
1. The learned constraints are generalized by abstraction over the time domain, which results in candidate properties.
1. The candidate properties are validated, using an induction-based proof method or a simpler, state-wise method.
1. Valid generalized constraints can finally be reused via generalized constraint feedback, that is, by enriching logic programs with the newly generalized constraints.

## Literature

* [Patrick Lühne](https://www.luehne.de), 2015. [*Generalizing Learned Knowledge in Answer Set Solving*](https://www.luehne.de/theses/generalizing-learned-knowledge-in-answer-set-solving.pdf). M.Sc. Thesis, Hasso Plattner Institute, Potsdam

## Building and Installation

`ginkgo` is built with `cmake` and requires a compiler capable of C++14 as well as `boost` and `jsoncpp`.
`ginkgo` requires [`clasp`](https://github.com/potassco/clasp), [`xclasp`](https://github.com/potassco/xclasp), and [`gringo`](https://github.com/potassco/clingo) binaries at runtime.
The optional graphical output requires `GTKmm`.

## Usage

`ginkgo` consists of the following tools:

* `ginkgo-produce` accepts a PDDL instance and domain as input (as ASP facts, such as obtained with [`plasp`](http://potassco.sourceforge.net/labs.html#plasp)) and generalizes a specified number of learned constraints.
* `ginkgo-consume` performs *generalized constraint feedback*. That is, a given is solved again but enriched with a specific number of constraints previously generalized with `ginkgo-produce`. `ginkgo-consume` was primarily implemented for benchmarking and demonstration purposes.
* `ginkgo-analyze` uses the statistical output of the above two tools and aggregates the results for easier evaluation.
* `ginkgo-evaluate` displays a graphical evaluation of the statistical analyses.

Use the option ```--help``` for a full list of options of each of the respective tools.

## Contributors

* [Patrick Lühne](https://www.luehne.de)
