## Hypest Database

The database used to keep track of match data and other things for Hypest tournaments. It also uses a Glicko-2 system which
powers the seeding of the tournaments and the rankings.

## Goals

The above is a hypothetical scenario and not quite real. But it'd be cool if it was real no?

So here are the current goals:

- Work for multiple games.
    - This means that Smash 4, Melee, Project M, etc all have different sets of data.
    - There are a couple ways of doing this. Different database files (e.g. `melee.json` and `smash4.json`) or
      storing everything in one mass database.
- A sane command line API.
    - This includes different commands to print ranking tables, update the Glicko system, etc.

## Protocol

The JSON protocol for the user list. Using challonge username "rapptz" as an example.

```js
{
    "rapptz": {
        "challonge_username": "rapptz",
        "reddit_username": "rapptz",
        "wins": 5,
        "losses": 10,
        "ties": 2,
        "ranking": 1500,
        "rd": 350,
        "vol": 0.06,
        "bttom_interval": 1690,
        "top_interval": 1204,
        "tournaments_won": 0
    }
}
```

## Dependencies

- libcurl >= 7.3

A C++14 compiler is required (such as GCC 4.9).

## License

MIT license.
