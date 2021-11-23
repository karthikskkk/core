## Turn/Token blockproducer scheduling algorithm

The algorithm which determines the order of blockproducers is referred
to as the _blockproducer scheduling algorithm_.

This was designed by a community bounty in thread

however, Graphene has an additional requirement which

is not taken into account by the solutions in the thread:

The membership and length of the list of blockproducers may change over
time.

So in this article I'll describe my solution.

## Turns and tokens

The solution is based on terms of _turns_ and _tokens_.

- Newly inserted blockproducers start out with a turn and a token.
- In order for a blockproducer to be scheduled, it must have a turn and a token.
- The scheduler maintains a FIFO of blockproducers without tokens.
- If no blockproducer has a turn, then the scheduler gives a turn to all blockproducers. This is called "emitting a turn."
- While less than half of the blockproducers have tokens, give a token to the first blockproducer in the FIFO and remove it from the FIFO.
- Schedule a blockproducer by picking randomly from all blockproducers with both a turn and token.
- When a blockproducer is scheduled, it loses its turn and token.

## The generic scheduler

The generic scheduler implements turns and tokens. It only depends
on the C++11 stdlib and boost (not even using fc). Types provided
by Graphene are template parameters.

## The generic far future scheduler

The far future scheduler is implemented with the following rules:

- Run until you emit a turn.
- Record all blockproducers produced.
- Run until you emit a second turn.
- The blockproducers produced between the emission of the first turn (exclusive)
  and emission of the second turn (inclusive) are called the _far future schedule_.

Then the schedule for the rest of time is determined by repeating
the future schedule indefinitely. The far future scheduler is required
to give the scheduling algorithm bounded runtime and memory usage even
in chains involving very long gaps.

## Slots

Due to dynamic block interval, we must carefully keep in mind
the difference between schedule slots and timestamps. A
_schedule slot number_ is a positive integer. A slot number of `n`
represents the `n`th next block-interval-aligned timestamp after
the head block.

Note that the mapping between slot numbers and timestamps will change
if the block interval changes.

## Scheduling blocks

When each block is produced, the blockchain must determine whether
the scheduler needs to be run. If fewer than `num_blockproducers` are
scheduled, the scheduler will run until `2*num_blockproducers` are scheduled.
A block in which the scheduler runs is called a _scheduling block_.

Changes in the set of active blockproducers do not modify the existing
schedule. Rather, they will be incorporated into new schedule entries
when the scheduler runs in the next scheduling block. Thus, a blockproducer
that has lost an election may still produce 1-2 blocks. Such a blockproducer
is called a _lame duck_.

## Near vs. far schedule

From a particular chain state, it must be possible to specify a
mapping from slots to blockproducers, called the _total blockproducer schedule_.
The total blockproducer schedule is partitioned into a prefix, called the
_near schedule_; the remainder is the _far schedule_.

When a block occurs, `n` entries are _drained_ (removed) from the head
of the total schedule, where `n` is the slot number of the new block
according to its parent block.

If the block is a scheduling block, the total schedule is further
transformed. The new near schedule contains `2*num_blockproducers` entries,
with the previous near schedule as a prefix. The rest of the near
schedule is determined by the current blockchain RNG.

The new far schedule is determined by running the far future scheduler,
as described above. The far future scheduler also obtains entropy
from the current blockchain RNG.

As an optimization, the implementation does not run the far future
scheduler until a far-future slot is actually queried. With this
optimization, the only circumstance under which validating nodes must
run the far future scheduler is when a block gap longer than `num_blockproducers`
occurs (an extremely rare condition).

## Minimizing impact of selective dropout

The ability of any single malicious blockproducer to affect the results of the
shuffle algorithm is limited because the RNG is based on bit commitment
of the blockproducers. However, a malicious blockproducer _is_ able to
refuse to produce a block. A run of `m` consecutively scheduled
malicious blockproducers can independently make `m` independent choices
of whether to refuse to produce a block. Basically they are able to
control `m` bits of entropy in the shuffle algorithm's output.

It is difficult-to-impossible to entirely eliminate "the last person
being evil" problem in trustless distributed RNG's. But we can at least
mitigate this vector by rate-limiting changes to the total blockproducer
schedule to a very slow rate.

If every block schedules a blockproducer, our adversary with `m` malicious
blockproducers gets `m` chances per round to selectively drop out in order
to manipulate the shuffle order, allowing `m` attacks per round.
If blockproducers are only scheduled once per round,
a selective dropout requires the malicious blockproducer to produce the
scheduling block, limiting the probability to `m/n` attacks per round.
