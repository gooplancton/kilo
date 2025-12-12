import { ExtensionContext, workspace } from 'vscode';

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {

	// If the extension is launched in debug mode then the debug server options are used
	// Otherwise the run options are used
	const serverOptions: ServerOptions = {
		run: { command: 'forth_ls', transport: TransportKind.stdio },
		debug: {
			command: 'forth_ls',
			transport: TransportKind.stdio,
		}
	};

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		documentSelector: [{ scheme: 'file', language: 'forth', pattern: '**/*.forth' }],
 		synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/*.forth')
        }
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'forthls',
		'Forth Language Server',
		serverOptions,
		clientOptions
	);

	// Start the client. This will also launch the server
	client.start();
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
