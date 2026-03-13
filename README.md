# Agentic Firmware Experiment: A 5-day coding expedition into ARM Cortex-M microcontrollers

[![Website](https://img.shields.io/badge/Website-Live-brightgreen.svg)](https://olofattemo.github.io/agentic-firmware-experiment/)

This repository hosts the blog articles, prompts and source code for my Spring 2026 expedition into the frontier of AI Agents and their reliability in firmware development.

Topics range from using using Agentic Coding CLIs like Claude Code, Gemini CLI and associated LLMs to genereate code for the NRF ARM Cortex-M series chips with NRF Connect SDK and Python as primary language for scaffolds.

*Read the series on the project [blog](https://olofattemo.github.io/agentic-firmware-experiment/posts/).

## Project Structure

This is a monorepo containing both the examples and the blog:

- `experiments/` - Full examples from the posts.
- `docs/` - The blog website.

## Local Development (Blog)

To run the blog locally, ensure you have [Hugo](https://gohugo.io/installation/) and [Go](https://go.dev/doc/install) (for Hugo Modules) installed.

```bash
cd docs
hugo server -D
```
Open `http://localhost:1313` in your browser.

## License

This project is licensed under the **Apache 2.0** license. This applies to all hardware configurations, experimental firmware, and blog content contained within this repository.

