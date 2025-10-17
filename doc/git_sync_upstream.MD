# Steps to Sync Local, Remote, and Upstream Repository

## Sync Local w/ Upstream
#### Fetch ALL upstream changes (all branches)
1. git fetch upstream
#### Checkout to main (we only want main to be the most up-to-date)
2. git checkout main
#### Merge the LOCAL main branch w/ the upstream main using default merge commit message
3. git merge upstream/main --no-edit

## Sync Remote w/ Local
#### Push the local main branch to remote (we do this before fetching as the local repo is the "middleman" to syncing the upstream w/ remote)
1. git push origin main
#### Fetch all potential remote changes on any non-main repository branches and sync remote-tracking branches (incase of deleted remote branch)
2. git fetch --all --prune
#### Finally, pull all remote changes
3. git pull
