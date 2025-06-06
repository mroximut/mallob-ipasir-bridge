# Mallob IPASIR Bridge
An IPASIR interface to connect applications to Mallob's incremental on-demand SAT solving

## Building

Execute `make`. This produces the library `libipasirmallob.a` which provides all IPASIR functions as defined in `src/ipasir.h`.
The interface will select randomly among the available API paths provided by Mallob in the files `/tmp/mallob.apipath.*`.

Now you can link IPASIR applications with the library file `libipasirmallob.a` to obtain an application which uses Mallob as a backend SAT solver.
For more information on linking IPASIR applications with IPASIR solvers, see [the IPASIR Github repository](https://github.com/biotomas/ipasir).

## Usage

Mallob must run in the background in order to use this interface.
For each call to `solve()`, this IPASIR bridge creates a JSON request file and puts it into the `new/` subdirectory of the API directory and awaits an answer in the `done/` subdirectory.

For correct functionality, it is essential that your application properly calls `ipasir_release` for each IPASIR instance it created.
Otherwise, stale job nodes that are reserved for the dead job may accumulate and eventually clog the entire system. 

## ToDos

* The termination method is not functional yet because Mallob does not yet support arbitrary interruption of a certain incremental job "from the outside".
* No clauses are reported even if a callback is supplied: Mallob does not export clauses to the outside (yet).


## Usage with CBMC

* Execeute `make` in this directory.
* Build cbmc in its own directory with the following command:
``` make -C src LIBS="<path_to_the_bridge>/mallob-ipasir-bridge/libipasirmallob.a" IPASIR="<path_to_the_bridge>/mallob-ipasir-bridge/src" ```
* Go to the mallob directory and run `build/mallob -t=1 -compress-models`
* Go to the cbmc directory and run `./src/cbmc/cbmc <to_be_verified>.c`
* While verifing some programs with cbmc, the resulting SAT instance might not have clauses but still assumptions. To handle that cases, one can disable the assert in `mallob/src/app/sat/execution/engine:132`.
