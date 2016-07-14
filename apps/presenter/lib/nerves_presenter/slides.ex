defmodule NervesPresenter.Slides do
  use GenServer

  # Public API
  def start_link do
    GenServer.start_link(__MODULE__, [], name: __MODULE__)
  end

  def set_url(url) do
    GenServer.cast __MODULE__, {:set_url, url}
  end

  def first do
    set_url(base_url())
  end

  # Go to the specified slide number.
  def goto(number) do
    set_url(base_url() <> Integer.to_string(number))
  end

  # gen_server callbacks
  def init(_args) do
    executable = :code.priv_dir(:nerves_presenter) ++ '/console'
    port = Port.open({:spawn_executable, executable}, [{:packet, 2}, :use_stdio, :binary])
    state = %{port: port}
    cast_port(state, :set_url, [base_url()])
    { :ok, state }
  end

  def handle_cast({:set_url, url}, state) do
    cast_port(state, :set_url, [url])
    {:noreply, state}
  end
  def handle_cast(:stop, state) do
    {:stop, :normal, state}
  end

  def handle_info({_, {:data, message}}, state) do
    msg = :erlang.binary_to_term(message)
    IO.puts "Unexpected message: " <> inspect(msg)
    {:noreply, state}
  end

  # Private helper functions
  defp cast_port(state, command, arguments) do
    msg = {command, arguments}
    send state.port, {self, {:command, :erlang.term_to_binary(msg)}}
  end

  defp call_port(state, command, arguments) do
    cast_port(state, command, arguments)
    receive do
      {_, {:data, response}} ->
        {:ok, :erlang.binary_to_term(response)}
        _ -> :error
    end
  end

  defp base_url do
    List.to_string('file://' ++ :code.priv_dir(:pljelixir) ++ '/html/index.html#/')
  end
end
