# This file is responsible for configuring your application
# and its dependencies with the aid of the Mix.Config module.
#
# This configuration file is loaded before any dependency and
# is restricted to this project.
use Mix.Config

config :nerves, :firmware,
  rootfs_additions: "config/rootfs-additions"

config :elixir,
  ansi_enabled: true
