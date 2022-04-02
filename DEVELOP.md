Releasing OpenPegasus
=====================

A release of OpenPegasus is maintained in the  'OpenPegasus/OpenPegasus'
github repository and defined with both a git branch and a git tag.

Note: Releases 2.14.1,2.14.2, and 2.14.3 were inconsistent with this
procecure

Git workflow
------------

* Long-lived branches:
  - `main` - for next functional version
  - `OpenPegasus_M.N` - for fix stream of released version `M.N`.
  - `OpenPegasus_M.N.U` - for minor fix stream of released version `M.N` where
    the branch was started from the main branch.  .
* We use topic branches for everything!
  - Based upon the intended long-lived branch, if no dependencies
  - Based upon an earlier topic branch, in case of dependencies
  - It is valid to rebase topic branches and force-push them.
* We use pull requests to review the branches.
  - Use the correct long-lived branch (e.g. `main` or `stable_0.8`) as a
    merge target!
  - Review happens as comments on the pull requests.
  - At least one is required for merging.
* GitHub meanwhile offers different ways to merge pull requests. We merge pull
  requests by creating merge commits, so the single commits of a topic branch
  remain unchanged, and we see the title line of the pull request in the merge
  commit message, which is often the only place that tells the issue that was
  fixed.

* For now because we have a very limited number of developers and will release
  whenever we have changes, the development branches are linear and all
  are based on the main branch. If we determine that we are going to do a
  significant release and there there may be required changes to the existing
  release before we want to release the next significant version, we may
  elect to create new update versions (M.N.Unew) based on the stable branch
  of its minor version.

Releasing a version
-------------------

This section describes how to release a version of OpenPegasus

It covers all variants of versions that can be released:

* Releasing a new major version (Mnew.0.0) based on the main branch
* Releasing a new minor version (M.Nnew.0) based on the main branch
* Releasing a new update version (M.N.Unew) based on the stable branch of its
  minor version

The description assumes that the `OpenPegasus/OpenPegasus` repo is cloned
locally in a directory named `OpenPegasus`. Its upstream repo is assumed to
have the remote name `origin`.

Any commands in the following steps are executed in the main directory of your
local clone of the `OpenPegasus/OpenPegasus` Git repo.

1.  Set shell variables for the version that is being released and the branch
    it is based on:

    * ``MNU`` - Full version M.N.U that is being released
    * ``MN`` - Major and minor version M.N of that full version
    * ``BRANCH`` - Name of the branch the version that is being released is
      based on

    When releasing a new major version (e.g. ``3.0.0``) based on the main
    branch:

        MNU=3.0
        MN=3.0
        BRANCH=main

    When releasing a new minor version (e.g. ``2.15.0``) based on the main
    branch:

        MNU=2.15.0
        MN=2.15
        BRANCH=main

    When releasing a new update version (e.g. ``2.14.4``) based on the main
    branch:

        MNU=2.14.4
        MN=2.14
        BRANCH=main

2.  Create a topic branch for the version that is being released:

        git checkout ${BRANCH}
        git pull
        git checkout -b release_${MNU}

3.  Edit the OpenPegasus file that define the version:

        vi OpenPegasus/pegasus/src/Pegasus/Common/pegasus_version.h

    and update the version number string. This file defines the current version
    of OpenPegasus and also the state (developement or production) of the
    current code. Note that the version must be set in multiple places in this
    file as a string and as an integer where each byte defines a part of the
    version.

    In addition a number of the test result files must be modified because the
    OpenPegasus version is hardcoded into these files. This is best accomplished
    by searching for files containing the version 'M.N.U' string.  These should
    be result master files and there will be several of them. Change the
    version string to the new 'M.N.U'. If these files are not changed, the
    pegasus tests will fail.

4.  Edit the change log:

        vi docs/changes.rst

    and make the following changes in the section of the version that is being
    released:

    * Finalize the version.
    * Change the release date to today's date.
    * Make sure that all changes are described.
    * Make sure the items shown in the change log are relevant for and
      understandable by users.
    * In the "Known issues" list item, remove the link to the issue tracker and
      add text for any known issues you want users to know about.
    * Remove all empty list items.

5.  When releasing based on the main branch, edit the GitHub workflow file
    ``cpp.yml``:

        vi .github/workflows/cpp.yml

    and in the ``on`` section, increase the version of the ``stable_*`` branch
    to the new stable branch ``stable_M.N`` created earlier:


        on:
          schedule:
            . . .
          push:
            branches: [ main, stable_M.N ]
          pull_request:
            branches: [ main, stable_M.N ]

6.  Commit your changes and push the topic branch to the remote repo:

        git commit -asm "Release ${MNU}"
        git push --set-upstream origin release_${MNU}

7.  On GitHub, create a Pull Request for branch ``release_M.N.U``. This will
    trigger the CI runs.

    Important: When creating Pull Requests, GitHub by default targets the
    ``main`` branch. When releasing based on a stable branch, you need to
    change the target branch of the Pull Request to ``stable_M.N``.

8.  On GitHub, close milestone ``M.N.U``.

9.  On GitHub, once the checks for the Pull Request for branch ``start_M.N.U``
    have succeeded, merge the Pull Request (no review is needed). This
    automatically deletes the branch on GitHub.

10. Add a new tag for the version that is being released and push it to
    the remote repo. Clean up the local repo:

        git checkout ${BRANCH}
        git pull
        git tag -f ${MNU}   <---- TODO
        git push -f --tags
        git branch -D release_${MNU}   <---- TODO

11. When releasing based on the main branch, create and push a new stable
    branch for the same minor version:

        git checkout -b stable_${MN}
        git push --set-upstream origin stable_${MN}
        git checkout ${BRANCH}

    Note that no GitHub Pull Request is created for any ``stable_*`` branch.


12. On GitHub, edit the new tag ``M.N.U``, and create a release description on
    it. This will cause it to appear in the Release tab.

    You can see the tags in GitHub via Code -> Releases -> Tags.


Starting a new version
----------------------

This section shows the steps for starting development of a new version of
OpenPegasus.

This section covers all variants of new versions:

* Starting a new major version (Mnew.0.0) based on the main branch
* Starting a new minor version (M.Nnew.0) based on the main branch
* Starting a new update version (M.N.Unew) based on the stable branch of its
  minor version

The description assumes that the `OpenPegasus/OPenPegasus` repo is cloned locally in
a directory named `OpenPegasus`. Its upstream repo is assumed to have the
remote name `origin`.

Any commands in the following steps are executed in the main directory of your
local clone of the `OpenPegasus/OpenPegasus` Git repo.

1.  Set shell variables for the version that is being started and the branch it
    is based on:

    * ``MNU`` - Full version M.N.U that is being started
    * ``MN`` - Major and minor version M.N of that full version
    * ``BRANCH`` -  Name of the branch the version that is being started is
      based on

    When starting a new major version (e.g. ``1.0.0``) based on the main
    branch:

        MNU=1.0.0
        MN=1.0
        BRANCH=main

    When starting a new minor version (e.g. ``0.9.0``) based on the main
    branch:

        MNU=0.9.0
        MN=0.9
        BRANCH=main

    When starting a new minor version (e.g. ``0.8.1``) based on the main
    branch:

        MNU=0.8.1
        MN=0.8
        BRANCH=main

2.  Create a topic branch for the version that is being started:

        git checkout ${BRANCH}
        git pull
        git checkout -b start_${MNU}

3.  Edit the version file:

        vi OpenPegasus/pegasus/src/Pegasus/Common/pegasus_version.h

    and update the version to a draft version of the version that is being
    started:

         'M.N.U.dev1'

4.  Edit the change log:

        vi docs/changes.rst

    and insert the following section before the top-most section:

        openpegasus M.N.U.dev1
        ----------------------

        This version contains all fixes up to version M.N-1.x.

        Released: not yet

        **Incompatible changes:**

        **Deprecations:**

        **Bug fixes:**

        **Enhancements:**

        **Cleanup:**

        **Known issues:**

        * See `list of open issues`_.

        .. _`list of open issues`: https://github.com/OpenPegasus/OpenPegasus/issues

5.  Commit your changes and push them to the remote repo:

        git commit -asm "Start ${MNU}"
        git push --set-upstream origin start_${MNU}

6.  On GitHub, create a Pull Request for branch ``start_M.N.U``.

    Important: When creating Pull Requests, GitHub by default targets the
    ``main`` branch.

7.  On GitHub, create a milestone for the new version ``M.N.U``.

    You can create a milestone in GitHub via Issues -> Milestones -> New
    Milestone.

8.  On GitHub, go through all open issues and pull requests that still have
    milestones for previous releases set, and either set them to the new
    milestone, or to have no milestone.

9.  On GitHub, once the checks for the Pull Request for branch ``start_M.N.U``
    have succeeded, merge the Pull Request (no review is needed). This
    automatically deletes the branch on GitHub.

10. Update and clean up the local repo:

        git checkout ${BRANCH}
        git pull
        git branch -D start_${MNU}
