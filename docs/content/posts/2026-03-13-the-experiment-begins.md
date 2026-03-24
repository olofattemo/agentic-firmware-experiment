---
title: "The Experiment Begins"
date: 2026-03-13T14:01:00Z
draft: false
tags: ["Agentic Workflows", "Firmware", "LLM", "ARM Cortex-M"]
summary: "This opening post introduces the 5-part series and discusses how agentic LLM coding is shifting our workflows to empower domain experts. It examines key challenges: the gap between vague natural-language prompts and precision firmware needs, how rapidly evolving interfaces cause LLMs to generate plausible but incorrect code, and the hard limits where LLMs currently aren't helpful. The nRF54L15 SPI errata example illustrates how subtle hardware differences can turn confident LLM output into silent data corruption."
---

## Introduction

Welcome to my spring 2026 blog series about agentic firmware development. This will be a 5-part series intended to document my findings in semi-weekly posts. I have allocated 5 days to develop my skills in this area and will publish my findings and reflections on this blog. Let's begin.

## How LLMs are changing the way we work

During my years as a Software Engineer, I have worked in various industries and roles, from semi-truck and e-scooter firmware to e-commerce frontends, HPC, data analysis, and IoT. Today, my role is in IT architecture, where aligning tech with business, maintaining the bird’s-eye view, and directing the development of systems across a corporate landscape is my main focus.

I do love hands-on technical challenges, but the speed and progress of systems and product development has, from my perspective, been mostly bounded by non-technical factors for years. Actual coding is not, and has perhaps never been, the bottleneck—even if a lot of effort goes into the coding part—retaining the need to keep software teams separate from architects, systems engineers, and domain experts. In larger organizations, this can be for managerial structure or a result of component and system sourcing strategies. There are some solid reasons, but the silos slow us down and work against our understanding of what to focus on.

However, with the recent advances in LLM-assisted agentic coding, things have changed. It is now possible for a single individual with sufficient domain knowledge to properly verify the AI outputs to amplify output from the actual coding phase, perhaps by as much as an order of magnitude. This has some interesting implications for how development can be organized in general. What it also does is allow domain experts to do complete development experiments in a few hours of their spare time.

As some of us marveled at the idea of short-circuiting the coding part of software, and tried and scoffed at LLM-generated outputs in 2024, we also got the term "vibe coding" in 2025. This term carries the sentiment that perhaps we can use this for PoCs, but it won’t be very efficient. Now in 2026, we are looking at a situation where frontline models are arguably qualitatively better than the average practitioner, and no longer working as an assistant, but as an agent that converts intent to solution—sometimes with minimal direction, sometimes with terrible results.

But while your LinkedIn feed makes it look like everybody is already a PhD with 10 years' experience in agentic coding and its implications for business and organizations, it is still very early. If you are old enough to remember the late nineties when the internet took off, you will expect the tooling to be "under construction" for years as the hype will crescendo then implode, then grow organically, and finally change our lives and our societies forever. For better and for worse.

As we navigate the rapidly evolving agentic coding landscape, what are the new challenges? I will elaborate on a few weak areas based on what I have seen so far and how they could relate to firmware development.

## Clarity of instructions vs. training corpus of frontline LLMs

LLMs are statistical language models, or if you prefer a more critical label: "guessing machines." However, since they are trained on such an enormous amount of data, they are getting very good at guessing. They can also check their guesses against adjacent concepts in a reasoning loop to increase the quality of what is inferred.

In an agentic setup (e.g., plan -> execute -> observe -> correct) where the LLMs are paired with a "harness" that helps curate the input, the LLMs gain access to various tools that can be used to live-verify the guesses, increasing the quality even more. Also critical is the quality of the input. The input must be good enough to invoke the relevant knowledge paths, but also be clear enough to decide what paths should be taken to generate the most suitable output.

For applications like creative writing or website design, where output can vary a lot and still be principally correct, this is less critical. But for areas where correctness lies in a narrow window, like firmware development, the promise of "vibe coding" is more likely to disappoint. The key here is being able to assess, for the specific area of application, what the LLM knows and what must be provided as input.

The forms of input are also important. Plain language is notoriously vague. Compared to code, the difference in precision of expression between a request in plain English and the inferred implementation in code is staggering. Don't forget that the delta between the two forms is inferred ("guessed") by the LLM.

The implications are both fascinating and scary. If we want more precision, we need to use a more formal language for our requirements, like an RFC-type document. Depending on the LLM, you may get a better result, but it's not guaranteed. It will still depend on the input and the data the LLM was trained on. In some cases, you may want to use a mathematical expression as a requirement. And in some cases, it will not be enough because an LLM is not deterministic anyway and we must add additional knowledge and tooling to the process.

## LLMs trained on evolving interfaces

When it comes to libraries, APIs, etc., some application areas evolve much faster than others. If you are trying to generate something that depends on an interface that has been updated frequently and recently, the LLMs will select the most likely options present in its training data at the model source data training cutoff date. It is very likely that this is no longer correct. Fortunately, this is often easy to correct, and it is one of the strengths of agentic flows vs. code from LLM chats. Agents can correct themselves.

In the firmware application, there will be many chip models with subtly different register behaviors and addressing. Code examples can look very similar but are critically different across an axis the LLM might not know about. Outdated code examples that depend on old libraries can also contribute to this.

Consider the following driver code that initializes SPI for a peripheral running at 4 MHz or less on the nRF series chips. On the relatively recent nRF54L15, the frequency control is a prescaler setting. On the nRF52832, it's an enum type. If the LLM mixes these up and tries to set the enum setting on the nRF54L15, the code will not compile because that field does not exist. But if it manages to correct the field issue as per below, the code will trigger [issue [8]](https://docs.nordicsemi.com/bundle/errata_nRF54L15_Rev2/page/ERR/nRF54L15/Rev2/latest/anomaly_L15_8.html) in the errata v1-v2, causing corruption of any command bytes with a value of >= 0x80h on the wire. The workaround is hidden in the errata or in the Zephyr SPIM driver code that comes with more recent versions of the SDK. Will the LLM figure this out? It’s a coin toss; heads or tails?

```c
// nRF52832 — correct implementation
NRF_SPIM0->CONFIG = (SPIM_CONFIG_CPHA_Leading << SPIM_CONFIG_CPHA_Pos) |   // CPHA = 0
                    (SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos); // CPOL = 0
NRF_SPIM0->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M4;  // 4 MHz — fixed enum
NRF_SPIM0->TXD.PTR = (uint32_t)&tx_buf;
NRF_SPIM0->TXD.MAXCNT = sizeof(tx_buf);
NRF_SPIM0->RXD.PTR = (uint32_t)&rx_buf;
NRF_SPIM0->RXD.MAXCNT = sizeof(rx_buf);
NRF_SPIM0->TASKS_START = 1;

// nRF54L15 — equivalent to above but with PRESCALER setting. Corrupts data on the SPI bus.
NRF_SPIM00->CONFIG = (SPIM_CONFIG_CPHA_Leading << SPIM_CONFIG_CPHA_Pos) |
                     (SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos);
NRF_SPIM00->PRESCALER = 32;  // 128 MHz / 32 = 4 MHz — this is > 2!
NRF_SPIM00->DMA.TX.PTR = (uint32_t)&tx_buf;
NRF_SPIM00->DMA.TX.MAXCNT = sizeof(tx_buf);;
NRF_SPIM00->DMA.RX.PTR = (uint32_t)&rx_buf;
NRF_SPIM00->DMA.RX.MAXCNT = sizeof(rx_buf);
NRF_SPIM00->TASKS_START = 1;
```

## The known unknowns that LLMs ignore

LLMs are guessing machines. There are some things that they don't know and cannot know, but they are inclined to give you a response regardless. Responses can look correct in the same way as educated guesses that are dead wrong can. Two examples:

### Control loops
Frontline LLMs can reason about control theory and implement basic regulators with filters and other protection measures. But for non-rudimentary cases, we must realize that LLMs cannot understand the physics of our plant on their own or know the details of how to calibrate our sensors or actuators. We are better off using other tooling when dealing with these cases.

### Pixel-perfect UIs
Layout on constrained displays must be meticulously arranged. Textual requirements don't really cut it here either. We need a design model that defines geometry, colors, fonts, and controls. If we have a precise model, we can generate the UI deterministically. It could suggest a range of choices for each attribute, perhaps. But the fact that the LLM will fail at this does not stop it from trying.

## Summary and next steps

So an LLM is obviously not the only tool we need for firmware development. Recognizing that, let's move on and see how we can harness LLMs in this field in ways that are actually helpful.

In the next post I will show an example of what a current state of the art agentic coding harness and LLM can do when asked to write firmware targeting the nRF54L15.
