import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import * as GAME from './Game/constants.js';
import {shallow, mount, render} from 'enzyme'

beforeEach(() => {
	// Reset mocked fetch calls
  fetch.resetMocks()
})

it('renders without crashing', () => {
  const div = document.createElement('div');
  ReactDOM.render(<App />, div);
});

it("Fetches state on #componentDidMount", async () => {

	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200})

	const app = shallow(<App />)

  await app.instance().componentDidMount()

	// Starts in idle
	expect(app.state()).toHaveProperty('state', GAME.State.IDLE);
});

it('Calls server to update state on change', () => {
	// Setup mock
	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200});

	// Create app
	const app = shallow(<App />)

	// Try state change
	app.instance().stateChangeHandler(GAME.State.PLAY);

	// Check server got right state
	expect(fetch).toBeCalledWith('/state', expect.objectContaining({body: ''+GAME.State.PLAY}));
});

it('Updates state according to what server says', () => {
	// Create app
	const app = shallow(<App />)

	// Setup mocks
	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200});
	spyOn(app.instance(), 'setState').and.callThrough();
	app.update();

	// Try state change - return promise to handle errors
	return app.instance().fetchState().then(() => {
			// Async check
			expect(fetch).toBeCalledWith('/state');
			expect(app.instance().setState).toBeCalled();
			expect(app.state()).toHaveProperty('state', GAME.State.PLAY);
		})
});

it('Gets game type and settings from server', () => {
	// Create app
	const app = shallow(<App />);

	// Setup mocks
	fetch.mockResponse(JSON.stringify({
		type: GAME.Type.DOMINATION,
		config: {
			score_limit: 100
		}
	}), {status: 200});
	spyOn(app.instance(), 'setState').and.callThrough();
	app.update();

	// Try state change - return promise to handle errors
	return app.instance().fetchGame().then(() => {
			// Async check
			expect(fetch).toBeCalledWith('/game');
			expect(app.instance().setState).toBeCalled();
			expect(app.state()).toHaveProperty('game.type', GAME.Type.DOMINATION);
	});
});
