# This Project has Moved!

You can find the new project page on [Codeberg](https://codeberg.org/slembcke/Chipmunk2D).

At least for now, I will be supporting the project on both sites, but eventually I plan to eventually drop GitHub entirely.

To migrate an existing checkout, you can use the following command, or make a new checkout of the project from the new location.

```
git remote set-url origin https://codeberg.org/slembcke/Chipmunk2D
git fetch origin master
git reset origin/master
```

You may additionally need to run `git checkout .` to cleanup the deleted files. *This will also discard your local changes if you have made any.*

# But _WHY_?

*sigh...* Roll your eyes at my principles if you must, but I do not want to be a willing participant in this grand LLM experiment.

I feel like the writing has been on the wall since Microsoft bought GitHub in 2018. Sure, they've done some nice Open Source things like .NET Core, VSCode, and... uh... Windows Terminal I guess? We've known for a while now what Microsoft thinks Open Source is for: selling it as a service while removing the license terms in a quasi-legal way. The site is even quasi-unusable now if you aren't logged in. You get get rate limited into oblivion, presumably because they are upset that other AI companies are scraping them. Incredible hypocrisy.

Meanwhile their CEO shares a quote, ["Either you have to embrace the AI, or you get out of your career"](https://web.archive.org/web/20250805051200/https://ashtom.github.io/developers-reinvented). He then [promptly quit](https://github.blog/news-insights/company-news/goodbye-github/) leaving GitHub to be managed by the CoreAI team. So yeah... that's what Microsoft thinks Open Source is for: Something that can be hoarded, mined for profit, and sold back to the people that made it. (They certainly don't train it on their own code, oh goodness no... Keep that stuff secret!) At the very same time they are arguing that [the public should be the ones paying for the development](https://github.blog/open-source/maintainers/we-need-a-european-sovereign-tech-fund/) of the Open Source code they train on. How convenient!

Then, in August 2025 the website for Chipmunk2D (the rigid body physics library I maintain) was knocked offline due to an absurd amount of AI crawler traffic. A story you'll see repeated [again and again](https://thelibre.news/foss-infrastructure-is-under-attack-by-ai-companies/). Isn't it just great when megacorps make you pay the fees it costs for them to rob you? It's just great! I love this future!

So yeah. I'm sure moving my projects somewhere else will hurt my discoverability, and I'll stop getting those sweet sweet stars that let me know people appreciate my work enough to click a button. In the end the AI scrapers are still probably going to find the code elsewhere, but I do **not** have to be a willing participant at least. Good riddance!

Also, if you are still reading my little rant, tell someone that makes a piece of software that you like! I've had someone tell me they used debugger.lua to help debug their model train setup, and people that use Chipmunk2D to prototype simulations for research. That's worth so much more than GitHub stars!
