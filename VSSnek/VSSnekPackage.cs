using Microsoft.VisualStudio.LanguageServer.Client;
using Microsoft.VisualStudio.Threading;
using Microsoft.VisualStudio.Utilities;
using StreamJsonRpc;
using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

[ContentType("sneklang")]
[Export(typeof(ILanguageClient))]
public class SnekLanguageClient : ILanguageClient
{
	public string Name => "Bar Language Extension";

	public IEnumerable<string> ConfigurationSections => null;

	public object InitializationOptions => null;

	public IEnumerable<string> FilesToWatch => null;

	public bool ShowNotificationOnInitializeFailed => true;

	public event AsyncEventHandler<EventArgs> StartAsync;
	public event AsyncEventHandler<EventArgs> StopAsync;

	public async Task<Connection> ActivateAsync(CancellationToken token)
	{
		await Task.Yield();

		ProcessStartInfo info = new ProcessStartInfo();
		info.FileName = "D:\\Dev\\2023\\CGL\\snek-lsp\\bin\\x64\\Debug\\snek-lsp.exe";
		info.Arguments = "";
		info.RedirectStandardInput = true;
		info.RedirectStandardOutput = true;
		info.UseShellExecute = false;
		info.CreateNoWindow = true;

		Process process = new Process();
		process.StartInfo = info;

		if (process.Start())
		{
			return new Connection(process.StandardOutput.BaseStream, process.StandardInput.BaseStream);
		}

		return null;
	}

	public async Task OnLoadedAsync()
	{
		await StartAsync.InvokeAsync(this, EventArgs.Empty);
	}

	public Task OnServerInitializeFailedAsync(Exception e)
	{
		return Task.CompletedTask;
	}

	public Task OnServerInitializedAsync()
	{
		return Task.CompletedTask;
	}

	public Task<InitializationFailureContext> OnServerInitializeFailedAsync(ILanguageClientInitializationInfo initializationState)
	{
		return (Task<InitializationFailureContext>)Task.CompletedTask;
	}
}
