# Contributing to Sming Framework
All contributions (PRs) to Sming Framework have to be done to the _develop_ branch. 

__master__: Branch that contains latest production (stable) release. No PRs other than Final Release updates will be merged into __master__.
__develop__: Main development branch: contains all current new features.

This means that __all contributors__ will have to submit a PR to a _develop_ , it will be tested and then merged to __develop__ for automated integration testing (via TravisCI, Jenkins or SemaphoreCI), as well as manual testing on a real device. 

__Sming Contributing flow__:
- Fork [_Sming_ repo](https://github.com/SmingHub/Sming#fork-destination-box) 

After that clone your own fork.

`git clone https://github.com/<my-fork>/Sming.git`

- Create a new branch off the _develop_ branch

```
cd Sming
git checkout develop
git branch feature/<short-explanation>
git checkout feature/<short-explanation>
```

Make sure to replace `short-explanation` with one or two words describing the purpose of the feature.
If you want to commit a fix use `fix/` as prefix for your branch name.

- Build, test your code

Make sure that your code compiles and it is tested on real device. Sometimes you will be asked for a proof (usually screenshot) that it was tested on your device(s). 

- Commit changes

```
git add <changed-files>
git commit -m "<Description of the change(s)>"
```

- Push your changes to your fork on github

```
git push
```

- Rebase if needed

If your branch cannot be merged automatically because there are new changes in the __develop__
branch that conflict with yours make sure to rebase your branch. The following commands can help
you do this

```
cd Sming
git checkout develop
git pull develop
git checkout feature/<short-explanation>
git merge develop
# Fix any merge conflicts if needed. 
git rebase develop
# Fix any merge conflicts if needed.  
```

If there were merge conflicts you will have to resolve them locally. This usually involves calling the following commands:

```
git mergetool
# After resolving conflicts you should type the command below to see what are the next steps
# Newer versions of `git` are kind enough to show hints
git status 
```

After that operation you should have a branch that has all latest changes from __develop__ 
with your changes on top. 


- Submit PR to the main Sming repo, __develop__ branch.
- Work with other contributors to test your feature and get it merged to _develop_

This is the most common approach for a git-flow:
http://nvie.com/posts/a-successful-git-branching-model/
