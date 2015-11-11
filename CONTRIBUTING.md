# Contributing to Sming Framework
All contributions (PRs) to Sming Framework have to be done to _develop_ branch. 
If a feature or bugfix is a major change please contact Sming team to prepare feature specific handling.

__master__: Branch that contains latest production (stable) release. No PRs other than Final Release updates will be merged into __master__.
__develop__: Main development branch: contains all current new features.

This will mean that __all contributors__ will have to submit a PR to a _develop_ , it will be tested and then merged to __develop__ for automated integration testing (via TravisCI, Jenkins or SemaphoreCI), as well as manual testing on a real device. 

__Sming Contributing flow__:
- Fork _Sming_ repo
- Create a branch off the _develop_: __MyNewFeature__
- Build, test your code
- Commit changes
- Push your changes to your fork on github
- Submit PR to the main Sming repo, __develop__ branch.
- Work with other contributors to test your feature and get it merged to _develop_

This is the most common approach for a git-flow:
http://nvie.com/posts/a-successful-git-branching-model/
