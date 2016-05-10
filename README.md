# ginkgo—Tools for Generalizing Learned Constraints

`ginkgo` uses ASP techniques to automatically generalize learned conflict constraints to make them applicable to other PDDL problem instances.

## Installation

`ginkgo` is built with `cmake` and requires a compiler capable of C++14 as well as `boost` and `jsoncpp`.
`ginkgo` requires `clasp`, `xclasp`, and `gringo` binaries at runtime.
The optional graphical output requires `GTKmm`.

## Usage

`ginkgo` consists of the following tools:

* `ginkgo-produce` accepts a PDDL instance and domain as input (as ASP facts, such as obtained with [plasp](http://potassco.sourceforge.net/labs.html#plasp)) and generalizes a specified number of learned constraints.
* `ginkgo-consume` performs *generalized constraint feedback*. That is, a given is solved again but enriched with a specific number of constraints previously generalized with `ginkgo-produce`. `ginkgo-consume` was primarily implemented for benchmarking and demonstration purposes.
* `ginkgo-analyze` uses the statistical output of the above two tools and aggregates the results for easier evaluation.
* `ginkgo-evaluate` displays a graphical evaluation of the statistical analyses.

Use the option ```--help``` for a full list of options of each of the respective tools.

## Contributors

`ginkgo` was implemented by Patrick Lühne from 2014 to 2016.
