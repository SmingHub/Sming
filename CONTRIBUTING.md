# Contributing to Sming Framework
All contributions (PRs) to Sming Framework have to be done to _develop_ branch. If a feature or bugfix is a major change a feature branch has to be created.


__master__: Branch that contains latest production (stable) release. No PRs other than release will be merged into __master__.
__develop__: Main development branch: contains all current new features.
__feature/websocket-client__. Gets deleted when merged to __develop__: Branch for a new feature while in development process. A feature branch is required when change is substential enough to be called a feature, or is backwards-incompatible with current code or environment and needs to be tested and rolled out appropriately.
__release/1.2.5__: Special release branch that exists only when develop gets merged to master. This branch is used when a feature-set in __develop__ is frozen, and the branch is used to send latest fixes, before it gets merged into master and published to [sming/releases](https://github.com/anakod/Sming/releases).

This will mean that __all contributors__ will have to submit a PR to a _develop_ or _feature_ branch, it will be tested and then merged to __develop__ for automated integration testing (via TravisCI, Jenkins or SemaphoreCI), as well as manual testing on a real device. 
__Proposed flow__:
- Fork _Sming_ repo
- Create a branch off the master: __feature/websocket-client__
- Build, test your code
- Commit changes
- Push your changes to your fork on github
- Submit PR to the main Sming repo, __develop__ or __feature/websocket-client__ branch.
- Work with other contributors to test your feature and get it merged to _develop_

This is the most common approach for a git-flow:
http://nvie.com/posts/a-successful-git-branching-model/