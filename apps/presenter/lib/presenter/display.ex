defmodule Presenter.Display do
  use GenServer

  def start_link(opts \\ []) do
    GenServer.start_link(__MODULE__, [], opts)
  end

  def init(_) do
    :timer.send_interval(1000, :refresh)
    {:ok, 0}
  end

  def terminate(_reason, _state) do
  end

  def handle_info(:refresh, state) do
    IO.puts "hello #{state}"
    {:noreply, state + 1}
  end
end
