# ginkgo—Tools for Generalizing Learned Constraints

[![GitHub Release](https://img.shields.io/github/release/potassco/ginkgo.svg?maxAge=3600)](https://github.com/potassco/ginkgo/releases)
[![Build Status](https://img.shields.io/travis/potassco/ginkgo/develop.svg?maxAge=3600&label=build (master))](https://travis-ci.org/potassco/ginkgo?branch=master)
[![Build Status](https://img.shields.io/travis/potassco/ginkgo/develop.svg?maxAge=3600&label=build (develop))](https://travis-ci.org/potassco/ginkgo?branch=develop)

`ginkgo` uses ASP techniques to automatically generalize learned conflict constraints to make them applicable to other PDDL problem instances.

`ginkgo` proceeds in a four-step approach:

1. Conflict constraints are extracted while solving a logic program with [`xclasp`](https://github.com/potassco/xclasp), a variant of [`clasp`](https://github.com/potassco/clasp) extended with facilities for logging learned constraints.
1. The learned constraints are generalized by abstraction over the time domain, which results in candidate properties.
1. The candidate properties are validated, using an induction-based proof method or a simpler, state-wise method.
1. Valid generalized constraints can finally be reused via *generalized constraint feedback*, that is, by enriching logic programs with the newly generalized constraints.

## Building and Installation

`ginkgo` is built with `cmake` and has the following dependencies:

* C++14 compiler
* [`libclingo`](https://github.com/potassco/clingo) ≥ 5.1
* `boost`
* `jsoncpp`
* `GTKmm` (optional, for graphical evaluation tool)

After installing the dependencies, build `ginkgo` like this:

```bash
$ git clone https://github.com/potassco/ginkgo.git
$ cd ginkgo
$ mkdir -p build/release
$ cd build/release
$ cmake ../.. -DCMAKE_BUILD_TYPE=Release
$ make
```

## Usage

`ginkgo` consists of the following tools:

* `ginkgo-produce` accepts a PDDL instance and domain as input (as ASP facts, such as obtained with [`plasp`](http://potassco.sourceforge.net/labs.html#plasp)) and generalizes a specified number of learned constraints.
* `ginkgo-consume` performs *generalized constraint feedback*. That is, a given is solved again but enriched with a specific number of constraints previously generalized with `ginkgo-produce`. `ginkgo-consume` was primarily implemented for benchmarking and demonstration purposes.
* `ginkgo-analyze` uses the statistical output of the above two tools and aggregates the results for easier evaluation.
* `ginkgo-evaluate` displays a graphical evaluation of the statistical analyses.

Use the option ```--help``` for a full list of options of each of the respective tools.

## Literature

* Martin Gebser, Roland Kaminski, Benjamin Kaufmann, [Patrick Lühne](https://www.luehne.de), Javier Romero, and Torsten Schaub: [*Answer Set Solving with Generalized Learned Constraints*](http://software.imdea.org/Conferences/ICLP2016/Proceedings/ICLP-TCs/p09-gebser.pdf). In: Technical Communications of the [32nd International Conference on Logic Programming](http://software.imdea.org/Conferences/ICLP2016/), 2016, pp. 9:1–9:14

* [Patrick Lühne](https://www.luehne.de): [*Generalizing Learned Knowledge in Answer Set Solving*](https://www.luehne.de/theses/generalizing-learned-knowledge-in-answer-set-solving.pdf). M.Sc. Thesis, 2015, Hasso Plattner Institute, Potsdam

## Contributors

* [Patrick Lühne](https://www.luehne.de)
* Torsten Schaub (encodings)
